#include "Map.hpp"

#include <json.hpp>
#include <fstream>
#include <thread>

#include "structures/treadWorkPool.hpp"

#include "console.hpp"

struct PoolState {
	const Json& jsonData;
	const std::filesystem::path& folderPath;
	std::atomic<size_t> index;
	std::vector<vulkan::ID>& sceneryIDs;

	PoolState(const Json& jsonData,
		const std::filesystem::path& folderPath,
		size_t index,
		std::vector<vulkan::ID>& sceneryIDs)
		: jsonData(jsonData)
		, folderPath(folderPath)
		, index(index)
		, sceneryIDs(sceneryIDs) {}

	PoolState(const PoolState& other)
		: jsonData(other.jsonData)
		, folderPath(other.folderPath)
		, index(other.index.load())
		, sceneryIDs(other.sceneryIDs) {}
};
struct ThreadState {
	vk::CommandPool commandPool;
};

ThreadState startUpFunction(PoolState& poolState) {
	(void)poolState;
	ThreadState threadState;

	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.queueFamilyIndex = vulkan::App::queueFamily;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer 
		| vk::CommandPoolCreateFlagBits::eTransient;

	threadState.commandPool = vulkan::App::device.createCommandPool(poolInfo);
	return threadState;
};

void cleanUpFunction(PoolState& poolState, ThreadState& threadState) {
	(void)poolState;
	vulkan::App::device.destroyCommandPool(threadState.commandPool);
};

bool updateFunction(PoolState& poolState, ThreadState& threadState) {
	size_t i = poolState.index.fetch_add(1);
	if (i >= poolState.jsonData["objects"].size())
		return false;
	auto& obj = poolState.jsonData["objects"][i];
	poolState.sceneryIDs[i] = vulkan::GameObjectContainer::Add(vulkan::GameObject{
		vulkan::ModelCache::loadModel(poolState.folderPath / obj["model"], threadState.commandPool),
		getVec3(obj["position"]),
		glm::quat(), glm::vec3(1.0),
		obj.contains("modelPosition") ? getVec3(obj["modelPosition"]) : glm::vec3(),
		obj.contains("modelRotation") ? getQuat(obj["modelRotation"]) : glm::quat(1, 0, 0, 0),
		obj.contains("modelScale") ? getVec3(obj["modelScale"]) : glm::vec3(1.0, 1.0, 1.0) },
		true
	);

	return true;
};

Map::~Map() noexcept {
	unload();
}

bool Map::load(std::filesystem::path folderPath) {
	if (!verifyMapFolder(folderPath))
		return false;

	std::ifstream file(folderPath / "config.json");
	if (!file) {
		Console::log(Console::Log::Type::error, "Failed to open config file: " + (folderPath / "config.json").string());
		return false;
	}
	Json jsonData;
	try {
		jsonData = Json::parse(file, nullptr, false, true);
	}
	catch (const Json::parse_error& e) {
		Console::log(Console::Log::Type::error, "Failed to parse config file: " + (folderPath / "config.json").string() + "\n Failed with error: " + e.what());
		return false;
	}
	if (!verifyConfigFile(jsonData, folderPath))
		return false;

	sceneryIDs = std::vector<vulkan::ID>(jsonData["objects"].size());

	lightSourcePos = glm::vec3(jsonData["lightSource"][0], jsonData["lightSource"][1], jsonData["lightSource"][2]);

	TreadWorkPool<PoolState, ThreadState> threadPool(8, 
		PoolState{ jsonData, folderPath, 0, sceneryIDs }, 
		updateFunction, startUpFunction, cleanUpFunction);

	threadPool.start();
	threadPool.waitForWork();

	try {
		threadPool.getExceptions();
	}
	catch (const std::exception& e) {
		Console::log(Console::Log::Type::error, "Failed to load map: " + folderPath.string() + "\n Failed with error: " + e.what());
		return false;
	}

	return true;
}

void Map::unload() {
	vulkan::GameObjectContainer::Remove(sceneryIDs);
	sceneryIDs.clear();
}

bool Map::verifyMapFolder(std::filesystem::path folderPath) const {
	if (!std::filesystem::exists(folderPath)) {
		Console::log(Console::Log::Type::error, "Map folder does not exist: " + folderPath.string());
		return false;
	}
	if (!std::filesystem::is_directory(folderPath)) {
		Console::log(Console::Log::Type::error, "Map folder is not a directory: " + folderPath.string());
		return false;
	}
	if (!std::filesystem::exists(folderPath / "config.json")) {
		Console::log(Console::Log::Type::error, "Map folder does not contain config.json: " + folderPath.string());
		return false;
	}
	return true;
}

bool Map::verifyConfigFile(Json jsonData, std::filesystem::path folderPath) const {
	bool valid = true;
	size_t errorCount = 0;

	auto errorHit = [&]() {
		valid = false;
		errorCount++;
	};

	if (!jsonData.contains("name")) {
		Console::log(Console::Log::Type::error, "Config file does not contain 'name' field");
		errorHit();
	}
	else if (!isString(jsonData["name"])) {
		Console::log(Console::Log::Type::error, "Config file 'name' field is not a string");
		errorHit();
	}

	if (!jsonData.contains("lightSource")) {
		Console::log(Console::Log::Type::error, "Config file does not contain 'lightSource' field");
		errorHit();
	}
	else if (!isVec3(jsonData["lightSource"])) {
		Console::log(Console::Log::Type::error, "Config file 'lightSource' field is not a vec3");
		errorHit();
	}

	if (!jsonData.contains("objects")) {
		Console::log(Console::Log::Type::error, "Config file does not contain 'objects' field");
		errorHit();
	}
	else if (!jsonData["objects"].is_array()) {
		Console::log(Console::Log::Type::error, "Config file 'objects' field is not an array");
		errorHit();
	}

	size_t index = 0;
	for (const Json& obj : jsonData["objects"]) {
		if (!obj.contains("model")) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": Does not contain 'model' field");
			errorHit();
		}
		else if (!isString(obj["model"])) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'model' field is not a string");
			errorHit();
		}
		else if (!std::filesystem::exists(folderPath / obj["model"].get<std::string>())) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'model' field does not point to an existing file: " + obj["model"].get<std::string>() + ", Full path: " + (folderPath / obj["model"].get<std::string>()).string());
			errorHit();
		}
		else if (!std::filesystem::is_regular_file(folderPath / obj["model"].get<std::string>())) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'model' field does not point to a regular file: " + obj["model"].get<std::string>() + ", Full path: " + (folderPath / obj["model"].get<std::string>()).string());
			errorHit();
		}
		else if (obj["model"].get<std::filesystem::path>().extension() != ".obj") {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'model' field does not have a valid extension, must be .obj: " + obj["model"].get<std::string>() + ", Full path: " + (folderPath / obj["model"].get<std::string>()).string());
			errorHit();
		}
		if (!obj.contains("position")) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": Does not contain 'position' field");
			errorHit();
		}
		else if (!isVec3(obj["position"])) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'position' field is not a vec3");
			errorHit();
		}
		if (obj.contains("modelPosition") && !isVec3(obj["modelPosition"])) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'modelPosition' field is not a vec3");
			errorHit();
		}
		if (obj.contains("modelRotation") && !isQuat(obj["modelRotation"])) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'modelRotation' field is not a quat");
			errorHit();
		}
		if (obj.contains("modelScale") && !isVec3(obj["modelScale"])) {
			Console::log(Console::Log::Type::error, "Object " + std::to_string(index) + ": 'modelScale' field is not a vec3");
			errorHit();
		}
		index++;
	}

	if (!valid) {
		Console::log(Console::Log::Type::message, "Config file in " + folderPath.string() + " is not valid. Found " + std::to_string(errorCount) + " errors.");
	}

	if (!jsonData.contains("description")) {
		Console::log(Console::Log::Type::message, "Consider adding a 'description' field to the config file");
	}

	return valid;
}