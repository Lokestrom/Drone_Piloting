#include "helpers.hpp"

#include <fstream>
#include <json/single_include/nlohmann/json.hpp>

using Json = nlohmann::json;

Drone getDrone(const char* dronePath) {
	Drone drone{};

	// TODO: error handling

	Json json = Json::parse(std::ifstream(std::string(dronePath) + "/config.json"), nullptr, true, true);

	drone.mass = json["mass"];
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			drone.inertiaTensor[i][j] = json["inertiaTensor"][i][j];
		}
	}

	for (const auto& engineJson : json["engines"]) {
		DroneEngine engine;
		engine.id = engineJson["id"];
		engine.position[0] = engineJson["position"][0];
		engine.position[1] = engineJson["position"][1];
		engine.position[2] = engineJson["position"][2];
		engine.forceDirection[0] = engineJson["forceDirection"][0];
		engine.forceDirection[1] = engineJson["forceDirection"][1];
		engine.forceDirection[2] = engineJson["forceDirection"][2];
		engine.maxThrust = engineJson["maxThrust"];
		drone.engines.push_back(engine);
	}

	return drone;
}