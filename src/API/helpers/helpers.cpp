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
		engine.direction[0] = engineJson["direction"][0];
		engine.direction[1] = engineJson["direction"][1];
		engine.direction[2] = engineJson["direction"][2];
		engine.maxThrust = engineJson["maxThrust"];
		drone.engines.push_back(engine);
	}

	return drone;
}

UserInputHandler::UserInputHandler(const std::vector<Handle*>& handles) 
	: handles(handles) {
}

namespace {
bool isValid(const UserInput& input, const std::vector<UserInputHandler::Handle*>& handles) noexcept {
	for (size_t i = 0; i < input.size; ++i) {
		auto checkName = [&input, &i](UserInputHandler::Handle* handle) {
			return input.names[i] == handle->name;
		};
		if (!std::ranges::any_of(handles, checkName)) {
			return false;
		}
	}
	
	for (const auto handle : handles) {
		auto checkName = [&handle](const char* inputName) {
			return handle->name == inputName;
		};
		if (!std::any_of(input.names, input.names + input.size, checkName)) {
			return false;
		}
	}

	return true;
}

}

void UserInputHandler::startUp(const UserInput& input) noexcept {
	assert(isValid(input, handles) && 
		"There is unaccounted for input, there is some inputs in the config not used by the program or vise versa"
		"Also check for spelling mistakes"
	);

	size_t buttonIndex = 0;
	size_t axisIndex = 0;

	for (size_t i = 0; i < input.size; i++) {
		for (auto handle : handles) {
			if (handle->name != input.names[i]) {
				continue;
			}
			
			if (input.types[i] == InputType::Button) {
				handle->valuePtr = &input.buttonPressed[buttonIndex++];
			}
			else {
				handle->valuePtr = &input.axisValues[axisIndex++];
			}
		}
	}

}

float UserInputHandler::HandleAxis1::getValue() const noexcept {
	assert(valuePtr && "Value pointer must be valid");
	assert((0 <= *static_cast<float*>(valuePtr) && *static_cast<float*>(valuePtr) <= 1) 
		&& "A float of axis 1 must have a value in the range [0,1]");
	return *static_cast<float*>(valuePtr);
}

float UserInputHandler::HandleAxis2::getValue() const noexcept {
	assert(valuePtr && "Value pointer must be valid");
	assert((-1 <= *static_cast<float*>(valuePtr) && *static_cast<float*>(valuePtr) <= 1)
		&& "A float of axis 2 must have a value in the range [-1,1]");
	return *static_cast<float*>(valuePtr);
}

ButtonStateCpp UserInputHandler::HandleButton::getValue() const noexcept {
	assert(valuePtr && "Value pointer must be valid");
	return *static_cast<ButtonStateCpp*>(valuePtr);
}