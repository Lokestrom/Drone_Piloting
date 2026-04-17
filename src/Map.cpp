#include "Map.hpp"

#include <json.hpp>
#include <fstream>

using Json = nlohmann::json;

static glm::vec3 getVec3(const Json& jsonObj) {
	return glm::vec3(jsonObj[0], jsonObj[1], jsonObj[2]);
}
static glm::quat getQuat(const Json& jsonObj) {
	return glm::quat(glm::vec3(
		glm::radians(static_cast<float>(jsonObj[0])),
		glm::radians(static_cast<float>(jsonObj[1])),
		glm::radians(static_cast<float>(jsonObj[2]))));
}
static glm::vec4 getVec4(const Json& jsonObj) {
	return glm::vec4{
		jsonObj[0],
		jsonObj[1],
		jsonObj[2],
		jsonObj[3],
	};
}

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
	sceneryIDs.reserve(jsonData["objects"].size());

	lightSourcePos = glm::vec3(jsonData["lightSource"][0], jsonData["lightSource"][1], jsonData["lightSource"][2]);

	for (auto& obj : jsonData["objects"]) {

		vulkan::Model3D::CreationTransform modelTransform {
			.position = obj.contains("modelPosition")
							? getVec3(obj["modelPosition"])
							: glm::vec3(),
			.scale = obj.contains("modelScale")
						 ? getVec3(obj["modelScale"])
						 : glm::vec3(1.0, 1.0, 1.0),
			.rotation = obj.contains("modelRotation")
							? getQuat(obj["modelRotation"])
							: glm::quat(1, 0, 0, 0),
			.color = obj.contains("modelColor")
						 ? getVec4(obj["modelColor"])
						 : glm::vec4()
		};

		sceneryIDs.push_back(
			vulkan::GameObjectContainer::Add(vulkan::GameObject{
				.model = vulkan::ModelCache::loadModel(folderPath / obj["model"], modelTransform),
				.position = glm::vec3(obj["position"][0], obj["position"][1], obj["position"][2]),
				.orientation = glm::quat() }));
	}
}

void Map::unload() {
	for (auto& id : sceneryIDs) {
		vulkan::GameObjectContainer::Remove(id);
	}
	sceneryIDs.clear();
}
