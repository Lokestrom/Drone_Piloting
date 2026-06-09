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

UserInputHandler::UserInputHandler(const std::vector<Handle*>& handles) 
	: handles(handles) {
}

bool UserInputHandler::isValid(const UserInput& input) const {
	for (size_t i = 0; i < input.buttonCount + input.axisCount; ++i) {
		bool found = false;
		for (const auto* handle : handles) {
			if (handle->name == input.names[i]) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

void UserInputHandler::update(const UserInput& input) {
	if (!input.changed) {
		return;
	}

	for (size_t i = 0; i < input.buttonCount; ++i) {
		for (auto* handle : handles) {
			if (handle->name == input.names[i]) {
				handle->type = Handle::Type::Button;
				handle->valuePtr = static_cast<void*>(&input.buttonPressed[i]);
			}
			// axis 2 way takes up two button slots, so we need to skip the next one if we encounter one
			if (input.types[i] == InputType::Axis2Way)
				i++;
		}
	}
	for (size_t i = 0; i < input.axisCount; ++i) {
		for (auto* handle : handles) {
			if (handle->name == input.names[i]) {
				handle->type = Handle::Type::Axis;
				handle->valuePtr = static_cast<void*>(&input.axisValues[i]);
			}
		}
	}
}

float UserInputHandler::HandleAxis1::getValue() const {
	if (type == Type::Button) {
		ButtonStateCpp state = *static_cast<ButtonStateCpp*>(valuePtr);
		return state == ButtonStateCpp::Up || state == ButtonStateCpp::Pressed;
	}
	return *static_cast<float*>(valuePtr) / 2. + 1.;
}

float UserInputHandler::HandleAxis2::getValue() const {
	if (type == Type::Button) {
		ButtonStateCpp state1 = *static_cast<ButtonStateCpp*>(valuePtr);
		ButtonStateCpp state2 = *(static_cast<ButtonStateCpp*>(valuePtr) + 1);
		return (state1 == ButtonStateCpp::Up || state1 == ButtonStateCpp::Pressed) - (state2 == ButtonStateCpp::Up || state2 == ButtonStateCpp::Pressed);
	}
	return *static_cast<float*>(valuePtr);
}

ButtonStateCpp UserInputHandler::HandleButton::getValue() const {
	if (type == Type::Axis) {
		return *static_cast<float*>(valuePtr) > 0.5 ? ButtonStateCpp::Pressed : ButtonStateCpp::Released;
	}
	return *static_cast<ButtonStateCpp*>(valuePtr);
}