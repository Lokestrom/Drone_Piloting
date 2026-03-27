#include "Drone.hpp"

#include <fstream>
#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "App.hpp"
#include "Settings.hpp"

using Json = nlohmann::json;

static glm::vec3 getVec3(const Json& jsonObj) {
	return glm::vec3(jsonObj[0], jsonObj[1], jsonObj[2]);
}
static glm::quat getQuat(const Json& jsonObj) {
	return glm::quat(glm::vec3(
		glm::radians(static_cast<float>(jsonObj[0])),
		glm::radians(static_cast<float>(jsonObj[1])),
		glm::radians(static_cast<float>(jsonObj[2]))
	));
}

static glm::vec4 getVec4(const Json& jsonObj) {
	return glm::vec4{
		jsonObj[0],
		jsonObj[1],
		jsonObj[2],
		jsonObj[3],
	};
}

Drone::Drone(std::filesystem::path folderPath) 
	: _mass(1) {

	std::ifstream file(folderPath / "config.json");
	assert(file && "Cant open config, callers responsibility to check");

	Json jsonData = Json::parse(file, nullptr, true, true);

	vulkan::Model3D::CreationTransform modelTransform {
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
		.model = vulkan::Model3D(folderPath / jsonData["model"], modelTransform),
		.position = glm::vec3(0.0),
		.orientation = glm::quat(1, 0, 0, 0)
	};

	objectID = vulkan::GameObjectContainer::Add(std::move(obj));
}

Drone::~Drone() noexcept {
	vulkan::GameObjectContainer::Remove(objectID);
}

static glm::quat getStepQuaternion(const glm::quat& orientation, const glm::quat& desiredOrientation, int n, float maxRotationSpeed) noexcept {
	glm::quat step = (desiredOrientation - orientation) / (float)n;
	float stepMagnitude = sqrt(step.x * step.x + step.y * step.y + step.z * step.z);
	if (stepMagnitude > maxRotationSpeed) {
		float scale = maxRotationSpeed / stepMagnitude;
		step.x *= scale;
		step.y *= scale;
		step.z *= scale;
	}
	return step;
}

void Drone::update() {
	auto& obj = vulkan::GameObjectContainer::get(objectID);

	glm::vec3 forces = glm::vec3(0.0, -10.0, 0.0);

	glm::vec3 forwardDir = glm::rotate(vulkan::GameObjectContainer::get(objectID).orientation, glm::vec3(0.0, 0.0, 1.0));
	glm::vec3 rightDir = glm::rotate(vulkan::GameObjectContainer::get(objectID).orientation, glm::vec3(1.0, 0.0, 0.0));
	glm::vec3 upDir = glm::rotate(vulkan::GameObjectContainer::get(objectID).orientation, glm::vec3(0.0, 1.0, 0.0));

	glm::vec3 moveDir{ 0.0 };
	moveDir += (float)ImGui::IsKeyDown(settings::moveForward) * forwardDir;
	moveDir -= (float)ImGui::IsKeyDown(settings::moveBackwards) * forwardDir;
	moveDir += (float)ImGui::IsKeyDown(settings::moveLeft) * rightDir;
	moveDir -= (float)ImGui::IsKeyDown(settings::moveRight) * rightDir;
	moveDir += (float)ImGui::IsKeyDown(settings::moveUp) * upDir;
	moveDir -= (float)ImGui::IsKeyDown(settings::moveDown) * upDir;

	if (moveDir != glm::vec3(0.0)) {
		forces += 20.0f * glm::normalize(moveDir);
	}

	//resistance
	forces += (glm::length(_velocity) / 10) * -_velocity;


	obj.position += _velocity * (float)App::getDeltaTime();
	_velocity += (forces / _mass) * (float)App::getDeltaTime();

	//faking the colition with the grownd
	if (obj.position.y < -10) {
		obj.position.y = -10;
		_velocity.y = 0;
	}

	/*glm::vec3 rotation{ 0.0 };
	glm::vec3 upRotate = glm::vec3(1.0, 0.0, 0.0);
	glm::vec3 rightRotate = glm::vec3(0.0, 1.0, 0.0);
	glm::vec3 rollRotate = glm::vec3(0.0, 0.0, 1.0);

	rotation += (float)ImGui::IsKeyDown(settings::rotateRight) * rightRotate;
	rotation -= (float)ImGui::IsKeyDown(settings::rotateLeft) * rightRotate;
	rotation += (float)ImGui::IsKeyDown(settings::rotateUp) * upRotate;
	rotation -= (float)ImGui::IsKeyDown(settings::rotateDown) * upRotate;
	rotation += (float)ImGui::IsKeyDown(settings::rollLeft) * rollRotate;
	rotation -= (float)ImGui::IsKeyDown(settings::rollRight) * rollRotate;

	float angle = glm::length(rotation) * App::getDeltaTime();
	if (angle < 1e-6f)
		return;

	glm::vec3 axis = glm::normalize(rotation);

	glm::quat dq = glm::angleAxis(angle, axis);

	getOrientation() = glm::normalize(getOrientation() * dq);*/
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
