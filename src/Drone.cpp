#include "Drone.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>

#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ImGui/imgui_internal.h>

#include "App.hpp"
#include "Settings.hpp"
#include "importJSONData.hpp"
#include "gui/settingsGui.hpp"
#include "console.hpp"

using Json = nlohmann::json;

using Axis2InputT = std::pair<ImGuiKey, ImGuiKey>;

constexpr std::string_view buttonTypeName = "button";
constexpr std::string_view axis1TypeName = "axis1";
constexpr std::string_view axis2TypeName = "axis2";

namespace {
[[nodiscard]]
ImGuiKey getKeyFromName(std::string name) {
	assert(!name.empty() && "input name is empty");
	name[0] = (char)std::toupper(name[0]);
	for (size_t i = 0; i < name.length() - 1; i++) {
		if (name[i] == ' ') {
			name.erase(i, 1);
			name[i] = (char)std::toupper(name[i]);
		}
	}
	for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
		if (ImGui::GetKeyName(static_cast<ImGuiKey>(i)) == name) {
			return static_cast<ImGuiKey>(i);
		}
	}
	assert(false && "Cant find the key");
	return ImGuiKey_None;
}

[[nodiscard]]
ButtonState getKeyButtonState(ImGuiKey key) {
	if (ImGui::IsKeyPressed(key, true))
		return ButtonState::Pressed;
	if (ImGui::IsKeyReleased(key))
		return ButtonState::Released;
	if (ImGui::IsKeyDown(key))
		return ButtonState::Down;

	return ButtonState::Up;
}

[[nodiscard]]
bool isAnalogKey(ImGuiKey key) noexcept {
	return key == ImGuiKey_GamepadL2 || key == ImGuiKey_GamepadR2 ||
			key == ImGuiKey_GamepadLStickLeft || key == ImGuiKey_GamepadLStickRight ||
			key == ImGuiKey_GamepadLStickUp || key == ImGuiKey_GamepadLStickDown ||
			key == ImGuiKey_GamepadRStickLeft || key == ImGuiKey_GamepadRStickRight ||
			key == ImGuiKey_GamepadRStickUp || key == ImGuiKey_GamepadRStickDown;
}

bool isAnalog2Way(ImGuiKey key) noexcept {
	return key == ImGuiKey_GamepadLStickLeft || key == ImGuiKey_GamepadLStickRight ||
		key == ImGuiKey_GamepadLStickUp || key == ImGuiKey_GamepadLStickDown ||
		key == ImGuiKey_GamepadRStickLeft || key == ImGuiKey_GamepadRStickRight ||
		key == ImGuiKey_GamepadRStickUp || key == ImGuiKey_GamepadRStickDown;
}

[[nodiscard]]
ImGuiKey getOpposingAxis(ImGuiKey key) noexcept {
	if (key % 2 == 0) {
		return (ImGuiKey)(key + 1);
	}
	return (ImGuiKey)(key - 1);
}

void setAxis1Way(const std::string& name, ImGuiKey& key) {
	static ImGuiKey* waitingForKey = nullptr;
	bool isWaiting = waitingForKey == &key;

	ImGui::TextUnformatted(name.c_str());
	ImGui::SameLine();

	std::string buttonText = ImGui::GetKeyName(key);
	if (isWaiting) {
		buttonText = "Press any key...";
	}

	ImVec2 buttonSize = ImGui::CalcTextSize(buttonText.c_str());
	buttonSize.x += ImGui::GetStyle().FramePadding.x * 10.0f;

	float availWidth = ImGui::GetContentRegionAvail().x;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - buttonSize.x);

	if (ImGui::Button(buttonText.c_str())) {
		if (isWaiting) {
			waitingForKey = nullptr;
			return;
		}
		waitingForKey = &key;
	}

	if (isWaiting) {
		for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
			ImGuiKey tempKey = static_cast<ImGuiKey>(i);
			if (ImGui::IsKeyReleased(tempKey)) {
				key = tempKey;
				waitingForKey = nullptr;
				break;
			}
			if (isAnalogKey(tempKey) && ImGui::GetKeyData(tempKey)->AnalogValue > 0.9) {
				key = tempKey;
				waitingForKey = nullptr;
				break;
			}
		}
	}
}

struct AxisBindingState {
	std::pair<ImGuiKey, ImGuiKey>* axis = nullptr;
	bool waitingForSecondKey = false;
};

void setAxis2Way(const std::string& name, std::pair<ImGuiKey, ImGuiKey>& keys) {
	static AxisBindingState bindingState;

	bool isWaiting =
		bindingState.axis == &keys;

	ImGui::TextUnformatted(name.c_str());
	ImGui::SameLine();

	std::string buttonText;

	if (!isWaiting) {
		buttonText =
			std::string(ImGui::GetKeyName(keys.first)) + " / " + ImGui::GetKeyName(keys.second);
	}
	else {
		buttonText = bindingState.waitingForSecondKey
						 ? "Press second key..."
						 : "Press key or axis...";
	}

	ImVec2 buttonSize = ImGui::CalcTextSize(buttonText.c_str());
	buttonSize.x += ImGui::GetStyle().FramePadding.x * 10.0f;

	float availWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - buttonSize.x);

	if (ImGui::Button(buttonText.c_str())) {
		if (isWaiting) {
			bindingState = {};
			return;
		}

		bindingState.axis = &keys;
		bindingState.waitingForSecondKey = false;
	}

	if (!isWaiting)
		return;

	for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; ++i) {
		ImGuiKey key = static_cast<ImGuiKey>(i);

		if (!bindingState.waitingForSecondKey && isAnalog2Way(key) &&
			ImGui::GetKeyData(key)->AnalogValue > 0.2f) {
			keys.first = key;
			keys.second = getOpposingAxis(key);

			bindingState = {};
			break;
		}

		if (!isAnalogKey(key) && ImGui::IsKeyReleased(key)) {
			if (!bindingState.waitingForSecondKey) {
				keys.first = key;
				bindingState.waitingForSecondKey = true;
			}
			else {
				keys.second = key;
				bindingState = {};
			}

			break;
		}
	}
}

}

Drone::Drone(std::filesystem::path folderPath)
	: _angularMomentum(0.0f)
	, _velocity(0.0f) {
	load(folderPath);
}

Drone::Drone(std::filesystem::path folderPath, API::DroneState state) {
	load(folderPath);

	getPosition() = glm::vec3{ state.position[0], state.position[1], state.position[2] };
	_velocity = glm::vec3{ state.velocity[0], state.velocity[1], state.velocity[2] };

	getOrientation() = glm::quat{ state.orientation[0], state.orientation[1], state.orientation[2], state.orientation[3] };
	glm::mat3 R = glm::mat3_cast(getOrientation());
	glm::mat3 _invInertia = R * _invInertia_B * glm::transpose(R);
	glm::vec3 angularVelocity = glm::rotate(getOrientation(), _invInertia * _angularMomentum);
	_angularMomentum = angularVelocity;
}

void Drone::load(std::filesystem::path folderPath) {
	std::ifstream file(folderPath / "config.json");
	assert(file && "Cant open config, callers responsibility to check");

	Json jsonData = Json::parse(file, nullptr, true, true);

	// TODO: should check for if the fields are of the correct type
	_mass = jsonData["mass"];
	_invInertia_B = glm::inverse(getMat3(jsonData["inertiaTensor"]));

	objectID = vulkan::GameObjectContainer::Add(vulkan::GameObject{
		vulkan::ModelCache::loadModel(folderPath / jsonData["model"]),
		// glm::vec3(2500.0, 0.0, -4500.0),
		glm::vec3(0, 0, 0),
		glm::quat(1, 0, 0, 0),
		glm::vec3(1.0),
		jsonData.contains("modelPosition") ? getVec3(jsonData["modelPosition"]) : glm::vec3(),
		jsonData.contains("modelRotation") ? getQuat(jsonData["modelRotation"]) : glm::quat(),
		jsonData.contains("modelScale") ? getVec3(jsonData["modelScale"]) : glm::vec3(1.0) });

	for (const auto& engineData : jsonData["engines"]) {
		Engines engine{
			.id = engineData["id"],
			.maxThrust = engineData["maxThrust"],
			.position = getVec3(engineData["position"]),
			.direction = glm::vec3(0, 1, 0)
		};
		_engines[engine.id] = engine;
	}

	auto& inputSettings = _settings.newCategory("Inputs");
	_inputType.reserve(jsonData["inputs"].size());

	std::vector<std::string> names;
	std::vector<const char*> namePtrs;
	names.reserve(jsonData["inputs"].size());
	namePtrs.reserve(jsonData["inputs"].size());
	for (const auto& inputData : jsonData["inputs"]) {
		std::string name = inputData["name"];
		names.push_back(name);
		namePtrs.push_back(names.back().c_str());
		if (inputData["type"].get<std::string>() == buttonTypeName) {
			inputSettings.emplace<settings::Value<ImGuiKey>>(name,
				getKeyFromName(inputData["default key"]), settings::Value<ImGuiKey>::setFunctionT(gui::keyBindButton));
			_inputButtonStates.push_back({});
			_inputType.push_back(API::InputType::Button);
		}
		else if (inputData["type"].get<std::string>() == axis1TypeName) {
			inputSettings.emplace<settings::Value<ImGuiKey>>(name,
				getKeyFromName(inputData["default key"]), settings::Value<ImGuiKey>::setFunctionT(setAxis1Way));
			_inputAxisStates.push_back(0.f);
			_inputType.push_back(API::InputType::Axis1Way);
		}
		else if (inputData["type"].get<std::string>() == axis2TypeName) {
			inputSettings.emplace<settings::Value<Axis2InputT>>(name,
				std::make_pair(getKeyFromName(inputData["default key"][0]), getKeyFromName(inputData["default key"][1])),
				settings::Value<Axis2InputT>::setFunctionT(setAxis2Way));
			_inputAxisStates.push_back(0.f);
			_inputType.push_back(API::InputType::Axis2Way);
		}
		else {
			Console::log(Console::Log::Type::error, std::string("The input: ") + name + 
				", had a wrong type: " + inputData["type"].get<std::string>());
		}
	}

	//Guarantee shrink since it will have a long life time
	std::vector<ButtonState>(_inputButtonStates).swap(_inputButtonStates);
	std::vector<float>(_inputAxisStates).swap(_inputAxisStates);

	_input.size = jsonData["inputs"].size();
	_input.names = namePtrs.data();
	_input.types = _inputType.data();
	_input.buttonPressed = reinterpret_cast<API::ButtonState*>(_inputButtonStates.data());
	_input.axisValues = _inputAxisStates.data();

	_plugin = {};
#ifdef _DEBUG
	_plugin.lib = SharedLib(folderPath / "Debug/control");
#else
	_plugin.lib = SharedLib(folderPath / "Release/control");
#endif
	_plugin.update = static_cast<API::UpdateFn>(_plugin.lib.getFunction("update"));
	if (_plugin.lib.hasFunction("getTargetPosition")) {
		_plugin.getTargetPosition = static_cast<API::GetTargetPositionFn>(_plugin.lib.getFunction("getTargetPosition"));
	}
	if (_plugin.lib.hasFunction("setup")) {
		static_cast<API::SetupFn>(_plugin.lib.getFunction("setup"))(folderPath.string().c_str(), &_input);
	}
	if (_plugin.lib.hasFunction("getSettings")) {
		_plugin.getSettings = static_cast<API::GetSettingsFn>(_plugin.lib.getFunction("getSettings"));
	}
}

Drone::~Drone() noexcept {
	vulkan::GameObjectContainer::Remove(objectID);
}

void Drone::update(bool active) {
	auto& obj = vulkan::GameObjectContainer::get(objectID);

	glm::mat3 R = glm::mat3_cast(getOrientation());
	glm::mat3 _invInertia = R * _invInertia_B * glm::transpose(R);
	glm::vec3 angularVelocity = glm::rotate(getOrientation(), _invInertia * _angularMomentum);

	API::DroneState state = getState();
	API::CommandBuffer commands{
		.commands = nullptr,
		.count = 0
	};

	populateInput();
	_plugin.update(&state, (float)App::getDeltaTime(), active, &commands);
	if (_plugin.getTargetPosition) {
		glm::vec3 targetPos;
		_plugin.getTargetPosition(&targetPos.x);
		::App::renderPoints.push_back({ targetPos, glm::vec4(1, 1, 0, 1) });
	}

	glm::vec3 forces = glm::rotate(glm::conjugate(getOrientation()), glm::vec3(0.0, -9.81, 0.0));
	glm::vec3 torque = glm::vec3(0.0, 0.0, 0.0);

	::App::renderVectors.push_back({ obj.position, glm::rotate(getOrientation(), forces), glm::vec4(0, 1, 0, 1) });
	for (int i = 0; i < commands.count; ++i) {
		const auto& command = commands.commands[i];

		if (_engines.find(command.engineId) == _engines.end()) {
			std::cerr << "Invalid engine ID: " << command.engineId << std::endl;
			continue;
		}
		if (std::abs(command.thrust) > _engines[command.engineId].maxThrust) {
			std::cerr << "Thrust value " << command.thrust << " exceeds max thrust for engine " << command.engineId << std::endl;
			continue;
		}
		forces += _engines[command.engineId].direction * command.thrust;
		torque += glm::cross(_engines[command.engineId].position, _engines[command.engineId].direction * command.thrust);

		::App::renderVectors.push_back({ glm::rotate(getOrientation(), _engines[command.engineId].position) + obj.position,
			glm::rotate(getOrientation(), _engines[command.engineId].direction * command.thrust) });
	}

	forces = glm::rotate(getOrientation(), forces);
	// forces -= (glm::length(_velocity) * _velocity) / 10.0f;
	::App::renderVectors.push_back({ obj.position, (glm::length(_velocity) * _velocity) / 10.0f, glm::vec4(0, 0, 1, 1) });

	obj.position += _velocity * (float)App::getDeltaTime();
	_velocity += (forces / _mass) * (float)App::getDeltaTime();

	_angularMomentum += torque * (float)App::getDeltaTime();
	//_angularMomentum -= (glm::length(_angularMomentum) * _angularMomentum) / 10.0f;

	angularVelocity = glm::rotate(getOrientation(), _invInertia * _angularMomentum);

	float angle = glm::length(angularVelocity) * (float)App::getDeltaTime();
	if (angle > 1e-6f) {
		glm::vec3 axis = glm::normalize(angularVelocity);

		glm::quat dq = glm::angleAxis(angle, axis);

		glm::quat newOrientation = glm::normalize(dq * getOrientation());

		if (glm::dot(newOrientation, getOrientation()) < 0.0f)
			newOrientation = -newOrientation;

		getOrientation() = newOrientation;
	}

	// faking the collision with the ground
	if (obj.position.y < -10 && _velocity.y < 0) {
		obj.position.y = -10;
		_velocity.y = 0;
	}
}

glm::vec3& Drone::getPosition() noexcept {
	return vulkan::GameObjectContainer::get(objectID).position;
}

const glm::vec3& Drone::getPosition() const noexcept {
	return vulkan::GameObjectContainer::get(objectID).position;
}

glm::quat& Drone::getOrientation() noexcept {
	return vulkan::GameObjectContainer::get(objectID).orientation;
}

const glm::quat& Drone::getOrientation() const noexcept {
	return vulkan::GameObjectContainer::get(objectID).orientation;
}

vulkan::GameObject& Drone::getObject() const noexcept {
	return vulkan::GameObjectContainer::get(objectID);
}

API::DroneState Drone::getState() const noexcept {
	auto& obj = getObject();

	glm::mat3 R = glm::mat3_cast(getOrientation());
	glm::mat3 _invInertia = R * _invInertia_B * glm::transpose(R);
	glm::vec3 angularVelocity = glm::rotate(getOrientation(), _invInertia * _angularMomentum);

	return API::DroneState{
		.position = { obj.position.x, obj.position.y, obj.position.z },
		.velocity = { _velocity.x, _velocity.y, _velocity.z },
		.orientation = { obj.orientation.w, obj.orientation.x, obj.orientation.y, obj.orientation.z },
		.angularVelocity = { angularVelocity.x, angularVelocity.y, angularVelocity.z }
	};
}

void Drone::populateInput() noexcept {
	_inputButtonStates.clear();
	_inputAxisStates.clear();

	const auto& values = _settings.get("Inputs").getValues();

	for (size_t i = 0; i < values.size(); i++) {
		switch (_inputType[i]) {
		case API::InputType::Button: {
			const auto& key = static_cast<settings::Value<ImGuiKey>*>(values[i].get())->getHandle().get();
			_inputButtonStates.push_back(getKeyButtonState(key));
		} break;
		case API::InputType::Axis1Way: {
			const auto& key = static_cast<settings::Value<ImGuiKey>*>(values[i].get())->getHandle().get();
			if (isAnalogKey(key)) {
				_inputAxisStates.push_back(ImGui::GetKeyData(key)->AnalogValue);
			}
			else {
				_inputAxisStates.push_back(ImGui::IsKeyDown(key));
			}
		} break;
		case API::InputType::Axis2Way: {
			const auto& keys = static_cast<settings::Value<Axis2InputT>*>(values[i].get())->getHandle().get();
			if (isAnalogKey(keys.first)) {
				_inputAxisStates.push_back(ImGui::GetKeyData(keys.first)->AnalogValue - ImGui::GetKeyData(keys.second)->AnalogValue);
			}
			else {
				_inputAxisStates.push_back((float)(ImGui::IsKeyDown(keys.first) - ImGui::IsKeyDown(keys.second)));
			}
		} break;
		default:
			assert(false && "Not all API input types are accounted for");
		}
	}
}
