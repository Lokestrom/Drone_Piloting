#include "Map.hpp"

#include <json.hpp>
#include <fstream>
#include <thread>
#include "importJSONData.hpp"

using Json = nlohmann::json;

Map::Map(std::filesystem::path folderPath) {
	load(folderPath);
}

Map::~Map() noexcept {
	unload();
}

void Map::load(std::filesystem::path folderPath) {
	std::ifstream file(folderPath / "config.json");
	assert(file && "Cant open map config, callers responsibility to check");

	Json jsonData = Json::parse(file, nullptr, true, true);
	sceneryIDs = std::vector<vulkan::ID>(jsonData["objects"].size());

	lightSourcePos = glm::vec3(jsonData["lightSource"][0], jsonData["lightSource"][1], jsonData["lightSource"][2]);

	std::atomic<size_t> index = 0;
	auto threadFunction = [&]() {
		while (true) {
			size_t i = index.fetch_add(1);
			if (i >= jsonData["objects"].size())
				return;
			auto& obj = jsonData["objects"][i];
			sceneryIDs[i] = vulkan::GameObjectContainer::Add(vulkan::GameObject{
				vulkan::ModelCache::loadModel(folderPath / obj["model"]),
				getVec3(obj["position"]),
				glm::quat(), glm::vec3(1.0),
				obj.contains("modelPosition") ? getVec3(obj["modelPosition"]) : glm::vec3(),
				obj.contains("modelRotation") ? getQuat(obj["modelRotation"]) : glm::quat(1, 0, 0, 0),
				obj.contains("modelScale") ? getVec3(obj["modelScale"]) : glm::vec3(1.0, 1.0, 1.0)
			}, true);
		}
	};

	std::array<std::thread, 1> threads;
	for (auto& thread : threads)
		thread = std::thread(threadFunction);

	for (auto& thread : threads)
		thread.join();
}

void Map::unload() {
	vulkan::GameObjectContainer::Remove(sceneryIDs);
	sceneryIDs.clear();
}
