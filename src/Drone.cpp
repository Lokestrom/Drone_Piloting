#include "Drone.hpp"

#include <fstream>
#include <iostream>

#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "App.hpp"
#include "Settings.hpp"
#include "importJSONData.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using Json = nlohmann::json;

struct droneControls {
	static const ImGuiKey moveForward = ImGuiKey_W,
						  moveBackward = ImGuiKey_S,
						  moveLeft = ImGuiKey_A,
						  moveRight = ImGuiKey_D,
						  moveUp = ImGuiKey_Space,
						  moveDown = ImGuiKey_LeftShift;
};

Drone::Drone(std::filesystem::path folderPath)
	: _angularMomentum(0.0f)
	, _velocity(0.0f)
{
	load(folderPath);
}

Drone::Drone(std::filesystem::path folderPath, DroneState state) {
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

	vulkan::Model3D::CreationTransform modelTransform{
		.position = jsonData.contains("modelPosition")
						? getVec3(jsonData["modelPosition"])
						: glm::vec3(),
		.scale = jsonData.contains("modelScale")
					 ? getVec3(jsonData["modelScale"])
					 : glm::vec3(1, 1, 1),
		.rotation = jsonData.contains("modelRotation")
						? getQuat(jsonData["modelRotation"])
						: glm::quat(),
		.color = jsonData.contains("modelColor")
					 ? getVec4(jsonData["modelColor"])
					 : glm::vec4()
	};

	vulkan::GameObject obj{
		.model = vulkan::ModelCache::loadModel(folderPath / jsonData["model"], modelTransform),
		.position = glm::vec3(0.0),
		.orientation = glm::quat(1, 0, 0, 0)
	};

	objectID = vulkan::GameObjectContainer::Add(std::move(obj));

	for (const auto& engineData : jsonData["engines"]) {
		Engines engine{
			.id = engineData["id"],
			.maxThrust = engineData["maxThrust"],
			.position = getVec3(engineData["position"]),
			.direction = glm::vec3(0, 1, 0)
		};
		_engines[engine.id] = engine;
	}
#ifdef _DEBUG
	_plugin.lib = SharedLib(folderPath / "Debug/control");
#else
	_plugin.lib = SharedLib(folderPath / "Release/control");
#endif
	_plugin.update = static_cast<UpdateFn>(_plugin.lib.getFunction("update"));
	if (_plugin.lib.hasFunction("getTargetPosition")) {
		_plugin.getTargetPosition = static_cast<GetTargetPositionFn>(_plugin.lib.getFunction("getTargetPosition"));
	}
	if (_plugin.lib.hasFunction("setup")) {
		static_cast<SetupFn>(_plugin.lib.getFunction("setup"))(folderPath.string().c_str());
	}
	if (_plugin.lib.hasFunction("getSettings")) {
		_plugin.getSettings = static_cast<GetSettingsFn>(_plugin.lib.getFunction("getSettings"));
	}
}

Drone::~Drone() noexcept {
	vulkan::GameObjectContainer::Remove(objectID);
}

static bool isnan(const glm::quat& q) noexcept {
	return std::isnan(q.x) || std::isnan(q.y) || std::isnan(q.z) || std::isnan(q.w);
}

static bool isnan(const glm::vec3& v) noexcept {
	return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
}

void Drone::update() {
	auto& obj = vulkan::GameObjectContainer::get(objectID);

	glm::mat3 R = glm::mat3_cast(getOrientation());
	glm::mat3 _invInertia = R * _invInertia_B * glm::transpose(R);
	glm::vec3 angularVelocity = glm::rotate(getOrientation(), _invInertia * _angularMomentum);

	UserInput input{
		.keyW = ImGui::IsKeyDown(droneControls::moveForward),
		.keyS = ImGui::IsKeyDown(droneControls::moveBackward),
		.keyA = ImGui::IsKeyDown(droneControls::moveLeft),
		.keyD = ImGui::IsKeyDown(droneControls::moveRight),
		.keySpace = ImGui::IsKeyDown(droneControls::moveUp),
		.keyShift = ImGui::IsKeyDown(droneControls::moveDown)
	};
	DroneState state = getState();
	CommandBuffer commands{
		.commands = nullptr,
		.count = 0
	};

	_plugin.update(&input, &state, (float)App::getDeltaTime(), &commands);
	if (_plugin.getTargetPosition) {
		glm::vec3 targetPos;
		_plugin.getTargetPosition(&targetPos.x);
		::App::renderPoints.push_back({ targetPos, glm::vec4(1, 1, 0, 1) });
	}

	glm::vec3 forces = glm::rotate(glm::conjugate(getOrientation()), glm::vec3(0.0, -10.0, 0.0));
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

		::App::renderVectors.push_back({ 
			glm::rotate(getOrientation(), _engines[command.engineId].position) + obj.position, 
			glm::rotate(getOrientation(), _engines[command.engineId].direction * command.thrust) 
		});
	}
	
	forces = glm::rotate(getOrientation(), forces);
	//forces -= (glm::length(_velocity) * _velocity) / 10.0f; // resistance
	::App::renderVectors.push_back({ obj.position, (glm::length(_velocity) * _velocity) / 10.0f, glm::vec4(0, 0, 1, 1) });

	obj.position += _velocity * (float)App::getDeltaTime();
	_velocity += (forces / _mass) * (float)App::getDeltaTime();

	_angularMomentum += torque * (float)App::getDeltaTime();
	//_angularMomentum -= (glm::length(_angularMomentum) * _angularMomentum) / 10.0f; // angular resistance

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

DroneState Drone::getState() const noexcept {
	auto obj = getObject();

	glm::mat3 R = glm::mat3_cast(getOrientation());
	glm::mat3 _invInertia = R * _invInertia_B * glm::transpose(R);
	glm::vec3 angularVelocity = glm::rotate(getOrientation(), _invInertia * _angularMomentum);

	return DroneState{
		.position = { obj.position.x, obj.position.y, obj.position.z },
		.velocity = { _velocity.x, _velocity.y, _velocity.z },
		.orientation = { obj.orientation.w, obj.orientation.x, obj.orientation.y, obj.orientation.z },
		.angularVelocity = { angularVelocity.x, angularVelocity.y, angularVelocity.z }
	};
}