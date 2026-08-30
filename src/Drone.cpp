#include "Drone.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <utility>

#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ImGui/imgui_internal.h>

#include "App.hpp"
#include "Settings.hpp"
#include "gui/settingsGui.hpp"
#include <renderer/helpers.hpp>
#include "console.hpp"

using Axis2InputT = std::pair<ImGuiKey, ImGuiKey>;

constexpr std::string_view buttonTypeName = "button";
constexpr std::string_view axis1TypeName = "axis1";
constexpr std::string_view axis2TypeName = "axis2";

namespace {
[[nodiscard]]
bool isInputTypeName(const std::string& type) noexcept {
	return type == buttonTypeName || type == axis1TypeName || type == axis2TypeName;
}

[[nodiscard]]
ImGuiKey getKeyFromName(std::string name) noexcept {
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

[[nodiscard]] 
std::string_view getLibPath() noexcept {
#ifdef _DEBUG
	return "Debug/control";
#else
	return "Release/control";
#endif
}

[[nodiscard]] 
std::string_view getLibExtention() noexcept {
#ifdef WIN32
	return ".dll";
#else
	return ".so";
#endif
}

}

Drone::Drone(Drone&& other) noexcept
	: _settings(std::move(other._settings))
	, objectID(std::exchange(other.objectID, 0))
	, _invInertia_B(other._invInertia_B)
	, _velocity(other._velocity)
	, _angularMomentum(other._angularMomentum)
	, _mass(other._mass)
	, _engines(std::move(other._engines))
	, _plugin(std::move(other._plugin))
	, _inputType(std::move(other._inputType))
	, _inputButtonStates(std::move(other._inputButtonStates))
	, _inputAxisStates(std::move(other._inputAxisStates)) {
}

Drone& Drone::operator=(Drone&& other) noexcept {
	if (this == &other)
		return *this;

	if (objectID != 0)
		renderer::GameObjectContainer::remove(objectID);

	objectID = std::exchange(other.objectID, 0);
	_invInertia_B = other._invInertia_B;
	_velocity = other._velocity;
	_angularMomentum = other._angularMomentum;
	_mass = other._mass;
	_engines = std::move(other._engines);
	_plugin = std::move(other._plugin);
	_inputType = std::move(other._inputType);
	_inputButtonStates = std::move(other._inputButtonStates);
	_inputAxisStates = std::move(other._inputAxisStates);
	_settings = std::move(other._settings);
	return *this;
}

bool Drone::load(const std::filesystem::path& folderPath) {
	return _load(folderPath);
}

bool Drone::load(const std::filesystem::path& folderPath, const API::DroneState& state) {
	if (!_load(folderPath)) {
		return false;
	}

	getPosition() = glm::vec3{ state.position[0], state.position[1], state.position[2] };
	_velocity = glm::vec3{ state.velocity[0], state.velocity[1], state.velocity[2] };

	getOrientation() = glm::quat{ state.orientation[0], state.orientation[1], state.orientation[2], state.orientation[3] };
	glm::vec3 angularVelocity = glm::vec3{ state.angularVelocity[0], state.angularVelocity[1], state.angularVelocity[2] };
	glm::mat3 R = glm::mat3_cast(getOrientation());
	glm::mat3 invInertiaWorld = R * _invInertia_B * glm::transpose(R);
	glm::vec3 unrotatedAngularVelocity = glm::rotate(glm::inverse(getOrientation()), angularVelocity);

	_angularMomentum = glm::inverse(invInertiaWorld) * unrotatedAngularVelocity;
	return true;
}

bool Drone::_load(const std::filesystem::path& folderPath) {
	assert(std::filesystem::is_directory(folderPath) && "folderPath is not a directory");
	assert(objectID == 0 && "There is already a drone loaded");
	
	objectID = 0;
	if (!verifyFolder(folderPath))
		return false;

	std::ifstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);
	file.open(folderPath / "config.json");
	
	Json jsonData;
	try {
		jsonData = Json::parse(file, nullptr, true, true);
	}
	catch (std::exception& e) {
		Console::log(Console::Log::Type::error, std::string("Failed to parse config JSON with: ") + e.what());
		return false;
	}
	
	if (!verifyConfigFile(jsonData, folderPath))
		return false;

	_mass = jsonData["mass"];
	_invInertia_B = glm::inverse(getMat3(jsonData["inertiaTensor"]));
	_velocity = glm::vec3(0.0);
	_angularMomentum = glm::vec3(0.0);

	auto commandPool = renderer::createCommandPool(
		renderer::App::queueFamily,
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
			vk::CommandPoolCreateFlagBits::eTransient);
	objectID = renderer::GameObjectContainer::Add(renderer::GameObject{
		renderer::ModelCache::loadModel(folderPath / jsonData["model"], *commandPool),
		glm::vec3(0, 0, 0),
		glm::quat(1, 0, 0, 0),
		glm::vec3(1.0),
		jsonData.contains("modelPosition") ? getVec3(jsonData["modelPosition"]) : glm::vec3(),
		jsonData.contains("modelRotation") ? getQuat(jsonData["modelRotation"]) : glm::quat(),
		jsonData.contains("modelScale") ? getVec3(jsonData["modelScale"]) : glm::vec3(1.0) });

	for (const auto& engineData : jsonData["engines"]) {
		Engines engine {
			.id = engineData["id"],
			.maxThrust = engineData["maxThrust"],
			.position = getVec3(engineData["position"]),
			.direction = getVec3(engineData["direction"])
		};
		_engines[engine.id] = engine;
	}

	auto& inputSettings = _settings.newCategory("Inputs");
	_inputType.reserve(jsonData["inputs"].size());

	std::vector<std::string> inputNames;
	std::vector<const char*> inputNamePtrs;
	inputNames.reserve(jsonData["inputs"].size());
	inputNamePtrs.reserve(jsonData["inputs"].size());
	for (const auto& inputData : jsonData["inputs"]) {
		std::string name = inputData["name"];
		inputNames.push_back(name);
		inputNamePtrs.push_back(inputNames.back().c_str());
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
			assert(false && "A invalid type name made it here, not good. Fix error handeling");
		}
	}

	//Guarantee shrink since it will have a long life time
	std::vector<ButtonState>(_inputButtonStates).swap(_inputButtonStates);
	std::vector<float>(_inputAxisStates).swap(_inputAxisStates);

	API::UserInput input{
		.size = jsonData["inputs"].size(),
		.names = inputNamePtrs.data(),
		.types = _inputType.data(),
		.buttonPressed = reinterpret_cast<API::ButtonState*>(_inputButtonStates.data()),
		.axisValues = _inputAxisStates.data()
	};

	_plugin = {};
#ifdef _DEBUG
	_plugin.lib = SharedLib(folderPath / "Debug/control");
#else
	_plugin.lib = SharedLib(folderPath / "Release/control");
#endif
	if (!verifyPlugin(_plugin.lib)) {
		return false;
	}
	_plugin.update = static_cast<API::UpdateFn>(_plugin.lib.getFunction("update"));
	if (_plugin.lib.hasFunction("getTargetPosition")) {
		_plugin.getTargetPosition = static_cast<API::GetTargetPositionFn>(_plugin.lib.getFunction("getTargetPosition"));
	}
	if (_plugin.lib.hasFunction("setup")) {
		static_cast<API::SetupFn>(_plugin.lib.getFunction("setup"))(
			folderPath.string().c_str(), &input);
	}
	if (_plugin.lib.hasFunction("getSettings")) {
		_plugin.getSettings = static_cast<API::GetSettingsFn>(_plugin.lib.getFunction("getSettings"));
	}

	return true;
}

Drone::~Drone() noexcept {
	if (objectID != 0)
		renderer::GameObjectContainer::remove(objectID);
}

void Drone::update(bool active) {
	auto& obj = renderer::GameObjectContainer::get(objectID);

	glm::mat3 R = glm::mat3_cast(getOrientation());
	glm::mat3 invInertia = R * _invInertia_B * glm::transpose(R);
	glm::vec3 angularVelocity = glm::rotate(getOrientation(), invInertia * _angularMomentum);

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
		::App::renderPoints.push_back({ targetPos });
	}

	glm::vec3 forces = glm::rotate(glm::conjugate(getOrientation()), glm::vec3(0.0f, -9.81f, 0.0f) * _mass);
	glm::vec3 torque = glm::vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < commands.count && commands.commands; ++i) {
		const auto& command = commands.commands[i];

		if (_engines.find(command.engineId) == _engines.end()) [[unlikely]] {
			Console::log(Console::Log::Type::error, std::string("Invalid engine ID: ") + std::to_string(command.engineId));
			continue;
		}
		if (std::abs(command.thrust) > _engines[command.engineId].maxThrust) [[unlikely]] {
			Console::log(Console::Log::Type::warning, 
				std::string("Thrust value ") + std::to_string(command.thrust) + " exceeds max thrust for engine " + std::to_string(command.engineId));
			continue;
		}
		forces += _engines[command.engineId].direction * command.thrust;
		torque += glm::cross(_engines[command.engineId].position, _engines[command.engineId].direction * command.thrust);
	}

	forces = glm::rotate(getOrientation(), forces);
	// forces -= (glm::length(_velocity) * _velocity) / 10.0f;

	obj.position += _velocity * (float)App::getDeltaTime();
	_velocity += (forces / _mass) * (float)App::getDeltaTime();

	_angularMomentum += torque * (float)App::getDeltaTime();
	//_angularMomentum -= (glm::length(_angularMomentum) * _angularMomentum) / 10.0f;

	angularVelocity = glm::rotate(getOrientation(), invInertia * _angularMomentum);

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

	// every render object refrencing the obj physics state must happen after the physics update
	::App::renderVectors.push_back({ obj.position, (glm::length(_velocity) * _velocity) / 10.0f });
	::App::renderVectors.push_back({ obj.position, glm::rotate(getOrientation(), forces) });
	for (int i = 0; i < commands.count && commands.commands; ++i) {
		const auto& command = commands.commands[i];
		::App::renderVectors.push_back({ glm::rotate(getOrientation(), _engines[command.engineId].position) + obj.position,
			glm::rotate(getOrientation(), _engines[command.engineId].direction * command.thrust) });
	}
}

glm::vec3& Drone::getPosition() noexcept {
	return renderer::GameObjectContainer::get(objectID).position;
}

const glm::vec3& Drone::getPosition() const noexcept {
	return renderer::GameObjectContainer::get(objectID).position;
}

glm::quat& Drone::getOrientation() noexcept {
	return renderer::GameObjectContainer::get(objectID).orientation;
}

const glm::quat& Drone::getOrientation() const noexcept {
	return renderer::GameObjectContainer::get(objectID).orientation;
}

renderer::GameObject& Drone::getObject() const noexcept {
	return renderer::GameObjectContainer::get(objectID);
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
			const ImGuiKey key = static_cast<ImGuiKey&>(
				*static_cast<settings::Value<ImGuiKey>*>(values[i].get()));
			_inputButtonStates.push_back(getKeyButtonState(key));
		} break;
		case API::InputType::Axis1Way: {
			const ImGuiKey key = static_cast<ImGuiKey&>(
				*static_cast<settings::Value<ImGuiKey>*>(values[i].get()));
			if (isAnalogKey(key)) {
				_inputAxisStates.push_back(ImGui::GetKeyData(key)->AnalogValue);
			}
			else {
				_inputAxisStates.push_back(ImGui::IsKeyDown(key));
			}
		} break;
		case API::InputType::Axis2Way: {
			const auto& keys = static_cast<Axis2InputT&>(
				*static_cast<settings::Value<Axis2InputT>*>(values[i].get()));
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
	assert(_inputButtonStates.capacity() == _inputButtonStates.size() &&
		   "There has been added button input states this is not alowd and it must stay the same size for the duration of its lifetime");
	assert(_inputAxisStates.capacity() == _inputAxisStates.size() &&
		   "There has been added axis input states this is not alowd and it must stay the same size for the duration of its lifetime");
}

bool Drone::verifyFolder(const std::filesystem::path& folderPath) const {
	bool valid = true;
	size_t errorCount = 0;

	auto errorHit = [&]() {
		valid = false;
		errorCount++;
	};

	if (!std::filesystem::is_directory(folderPath)) {
		Console::log(Console::Log::Type::error, "Drone folder does not exist: " + folderPath.string());
		return false;
	}
	if (!std::filesystem::exists(folderPath / "config.json")) {
		Console::log(Console::Log::Type::error, "Drone folder does not contain config.json: " + folderPath.string());
		errorHit();
	}
	if (!std::filesystem::exists((folderPath / getLibPath()).string() + std::string(getLibExtention()))) {
		// How is string + string_view only introduced in 26
		Console::log(Console::Log::Type::error, 
			std::string("Drone folder does not contain a control script '") + 
			std::string(getLibPath()) + std::string(getLibExtention()) + "': " + folderPath.string()); 
		errorHit();
	}
	if (!valid) {
		Console::log(Console::Log::Type::message, "Drone folder " + folderPath.string() + " is not valid. Found " + std::to_string(errorCount) + " errors.");
	}

	return valid;
}

bool Drone::verifyConfigFile(const Json& jsonData, const std::filesystem::path& folderPath) const {
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
	else if (jsonData["name"].get<std::string>().empty()) {
		Console::log(Console::Log::Type::warning, "Drone name is empty");
	}

	if (jsonData.contains("model")) {
		if (!isString(jsonData["model"])) {
			Console::log(Console::Log::Type::error, "Config file 'model' field is not a string");
			errorHit();
		}
		else {
			if (!std::filesystem::is_regular_file(folderPath / jsonData["model"].get<std::string>())) {
				Console::log(Console::Log::Type::error, "Drone config file 'model' field does not point to a file: " + jsonData["model"].get<std::string>() + ", Full path: " + (folderPath / jsonData["model"].get<std::string>()).string());
				errorHit();
			}
			else if (jsonData["model"].get<std::filesystem::path>().extension() != ".obj") {
				Console::log(Console::Log::Type::error, "Drone config file 'model' field does not have a valid extension, must be .obj: " + jsonData["model"].get<std::string>() + ", Full path: " + (folderPath / jsonData["model"].get<std::string>()).string());
				errorHit();
			}
		}
	}
	else if(!std::filesystem::is_regular_file(folderPath / "model.obj")) {
		Console::log(Console::Log::Type::error, "Drone config file has no field 'model' using default name model.obj, but found no model.obj file");
		errorHit();
	}

	if (!jsonData.contains("mass")) {
		Console::log(Console::Log::Type::error, "Drone config file does not contain 'mass' field");
		errorHit();
	}
	else if (!isNumber(jsonData["mass"])) {
		Console::log(Console::Log::Type::error, "Drone config file 'mass' field is not a number");
		errorHit();
	}
	else if (jsonData["mass"].get<float>() <= 0.0f) {
		Console::log(Console::Log::Type::error, "Drone config file 'mass' field must be greater than 0");
		errorHit();
	}

	if (!jsonData.contains("inertiaTensor")) {
		Console::log(Console::Log::Type::error, "Drone config file does not contain 'inertiaTensor' field");
		errorHit();
	}
	else if (!isMat3(jsonData["inertiaTensor"])) {
		Console::log(Console::Log::Type::error, "Drone config file 'inertiaTensor' field is not a mat3");
		errorHit();
	}
	else if (std::abs(glm::determinant(getMat3(jsonData["inertiaTensor"]))) <= 1e-6f) {
		Console::log(Console::Log::Type::error, "Drone config file 'inertiaTensor' field must be invertible");
		errorHit();
	}

	if (jsonData.contains("modelPosition") && !isVec3(jsonData["modelPosition"])) {
		Console::log(Console::Log::Type::error, "Drone config file 'modelPosition' field is not a vec3");
		errorHit();
	}
	if (jsonData.contains("modelRotation") && !isQuat(jsonData["modelRotation"])) {
		Console::log(Console::Log::Type::error, "Drone config file 'modelRotation' field is not a quat");
		errorHit();
	}
	if (jsonData.contains("modelScale") && !isVec3(jsonData["modelScale"])) {
		Console::log(Console::Log::Type::error, "Drone config file 'modelScale' field is not a vec3");
		errorHit();
	}

	if (!jsonData.contains("engines")) {
		Console::log(Console::Log::Type::error, "Drone config file does not contain 'engines' field");
		errorHit();
	}
	else if (!jsonData["engines"].is_array()) {
		Console::log(Console::Log::Type::error, "Drone config file 'engines' field is not an array");
		errorHit();
	}
	else if (jsonData["engines"].empty()) {
		Console::log(Console::Log::Type::error, "Drone config file 'engines' field must contain at least one engine");
		errorHit();
	}
	else {
		std::unordered_set<uint64_t> engineIds;
		size_t index = 0;
		for (const Json& engine : jsonData["engines"]) {
			if (!engine.is_object()) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": Is not an object");
				errorHit();
				index++;
				continue;
			}

			if (!engine.contains("id")) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": Does not contain 'id' field");
				errorHit();
			}
			else if (!engine["id"].is_number_unsigned()) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": 'id' field is not a non-negative integer");
				errorHit();
			}
			else {
				uint64_t id = engine["id"];
				if (!engineIds.insert(id).second) {
					Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": Duplicate engine id " + std::to_string(id));
					errorHit();
				}
			}

			if (!engine.contains("maxThrust")) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": Does not contain 'maxThrust' field");
				errorHit();
			}
			else if (!isNumber(engine["maxThrust"])) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": 'maxThrust' field is not a number");
				errorHit();
			}
			else if (engine["maxThrust"].get<float>() < 0.0f) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": 'maxThrust' field must be greater than or equal to 0");
				errorHit();
			}

			if (!engine.contains("position")) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": Does not contain 'position' field");
				errorHit();
			}
			else if (!isVec3(engine["position"])) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": 'position' field is not a vec3");
				errorHit();
			}

			if (!engine.contains("direction")) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": Does not contain 'direction' field");
				errorHit();
			}
			else if (!isVec3(engine["direction"])) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": 'direction' field is not a vec3");
				errorHit();
			}
			else if (glm::length2(getVec3(engine["direction"])) == 0) {
				Console::log(Console::Log::Type::error, "Engine " + std::to_string(index) + ": 'direction' vector must have length > 0");
				errorHit();
			}
			index++;
		}
	}

	if (jsonData.contains("inputs") && !jsonData["inputs"].is_array()) {
		Console::log(Console::Log::Type::error, "Drone config file 'inputs' field is not an array");
		errorHit();
	}
	else if (jsonData.contains("inputs")) {
		size_t index = 0;
		for (const Json& input : jsonData["inputs"]) {
			if (!input.is_object()) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": Is not an object");
				errorHit();
				index++;
				continue;
			}

			if (!input.contains("name")) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": Does not contain 'name' field");
				errorHit();
			}
			else if (!isString(input["name"])) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'name' field is not a string");
				errorHit();
			}
			else if (input["name"].get<std::string>().empty()) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'name' field must not be empty");
				errorHit();
			}

			if (!input.contains("type")) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": Does not contain 'type' field");
				errorHit();
			}
			else if (!isString(input["type"])) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'type' field is not a string");
				errorHit();
			}
			else if (!isInputTypeName(input["type"].get<std::string>())) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'type' field must be button, axis1, or axis2");
				errorHit();
			}

			if (!input.contains("default key")) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": Does not contain 'default key' field");
				errorHit();
			}
			else if (input.contains("type") && isString(input["type"]) && input["type"].get<std::string>() == axis2TypeName) {
				if (!input["default key"].is_array() || input["default key"].size() != 2 || !isString(input["default key"][0]) || !isString(input["default key"][1])) {
					Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'default key' field must be an array of two strings for axis2 inputs");
					errorHit();
				}
				else {
					if (getKeyFromName(input["default key"][0].get<std::string>()) == ImGuiKey_None) {
						Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'default key' field 1 contains an unknown key");
						errorHit();
					}

					if( getKeyFromName(input["default key"][1].get<std::string>()) == ImGuiKey_None) {
						Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'default key' field 2 contains an unknown key");
						errorHit();
					}
				}
			}
			else if (!isString(input["default key"])) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'default key' field is not a string");
				errorHit();
			}
			else if (getKeyFromName(input["default key"].get<std::string>()) == ImGuiKey_None) {
				Console::log(Console::Log::Type::error, "Input " + std::to_string(index) + ": 'default key' field contains an unknown key");
				errorHit();
			}
			index++;
		}
	}

	if (!valid) {
		Console::log(Console::Log::Type::message, "Drone config file in " + folderPath.string() + " is not valid. Found " + std::to_string(errorCount) + " errors.");
	}

	return valid;
}

bool Drone::verifyPlugin(const SharedLib& pluginLib) const {
	if (!pluginLib.isValid()) {
		Console::log(Console::Log::Type::error, "Drone script has not loaded correctly, error: " + pluginLib.getError());
		return false;
	}
	if (!pluginLib.hasFunction("update")) {
		Console::log(Console::Log::Type::error, "Drone script has no function 'update'. OS error: " + pluginLib.getError());
		return false;
	}
	return true;
}
