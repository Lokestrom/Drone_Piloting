#pragma once

#include "../../../../src/API/DroneAPI.h"
#include "../../../../src/API/helpers/helpers.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Eigen/Dense>

#include <cmath>
#include <optional>
#include <string>
#include <vector>

#ifndef ENGINE_COUNT
#error ENGINE_COUNT must be defined
#endif

static constexpr float GRAVITY = 9.81f;

struct PID {
	glm::vec3 integral = glm::vec3(0);

	glm::vec3 update(
		const glm::vec3& error,
		float kp,
		float ki,
		float dt) noexcept {
		integral += error * dt;
		return kp * error + ki * integral;
	}
};

struct ControllerState {
	PID positionPID;
	PID velocityPID;
	PID attitudePID;
};

static ControllerState g_controller;

static Drone g_drone;
static bool g_initialized = false;

static glm::vec3 g_targetPosition = glm::vec3(0);

static std::array<EngineCommand, ENGINE_COUNT> g_engineCommands;

static float POS_KP = 1.0f;
static float POS_KI = 0.0f;

static float VEL_KP = 2.0f;
static float VEL_KI = 0.05f;
static float VEL_KD = 1.5f;

static float ATT_KP = 8.f;
static float ATT_KI = .5f;

static float RATE_KP = 2.f;
static float RATE_KD = 3.f;

static float MOVE_SPEED = 20.0f;
static float ALTITUDE_SPEED = 10.0f;

static std::vector<const char*> g_settingNames = {
	"POS_KP",
	"POS_KI",

	"VEL_KP",
	"VEL_KI",
	"VEL_KD",

	"ATT_KP",
	"ATT_KI",

	"RATE_KP",
	"RATE_KD",

	"MOVE_SPEED",
	"ALTITUDE_SPEED"
};

static std::vector<float*> g_settingValues = {
	&POS_KP,
	&POS_KI,

	&VEL_KP,
	&VEL_KI,
	&VEL_KD,

	&ATT_KP,
	&ATT_KI,

	&RATE_KP,
	&RATE_KD,

	&MOVE_SPEED,
	&ALTITUDE_SPEED
};

namespace InputHandles {
static UserInputHandler::HandleAxis2 forward{ "Forward" };
static UserInputHandler::HandleAxis1 right{ "Right" };
static UserInputHandler::HandleAxis1 left{ "Left" };
static UserInputHandler::HandleButton up{ "Up" };
static UserInputHandler::HandleButton down{ "Down" };
}
static UserInputHandler g_inputHandler({ 
	&InputHandles::forward,
	&InputHandles::right,
	&InputHandles::left,
	&InputHandles::up,
	&InputHandles::down
});

static SettingsBuffer g_settings;

DRONE_API SettingsBuffer* getSettings() {
	return &g_settings;
}

static glm::vec3 clampMagnitude(
	const glm::vec3& v,
	float maxMag) noexcept {
	float len = glm::length(v);

	if (len <= maxMag)
		return v;

	return v * (maxMag / len);
}

static void solveSystem(
	const glm::vec3& desiredForce,
	const glm::vec3& desiredTorque) noexcept {
	Eigen::Matrix<float, 6, ENGINE_COUNT> A{};

	for (int i = 0; i < ENGINE_COUNT; ++i) {
		const DroneEngine& engine = g_drone.engines[i];

		A(0, i) = engine.forceDirection.x;
		A(1, i) = engine.forceDirection.y;
		A(2, i) = engine.forceDirection.z;

		glm::vec3 torqueAxis =
			glm::cross(engine.position, engine.forceDirection);

		A(3, i) = torqueAxis.x;
		A(4, i) = torqueAxis.y;
		A(5, i) = torqueAxis.z;
	}

	Eigen::Vector<float, 6> b;

	b << desiredForce.x,
		desiredForce.y,
		desiredForce.z,
		desiredTorque.x,
		desiredTorque.y,
		desiredTorque.z;

	Eigen::Matrix<float, ENGINE_COUNT, 6> AT = A.transpose();
	Eigen::Matrix<float, ENGINE_COUNT, ENGINE_COUNT> ATA = AT * A;

	constexpr float lambda = 0.000001f;

	for (int i = 0; i < ENGINE_COUNT; ++i)
		ATA(i, i) += lambda;

	Eigen::Vector<float, ENGINE_COUNT> thrusts =
		ATA.inverse() * (AT * b);

	for (int i = 0; i < ENGINE_COUNT; ++i) {

		float thrust =
			glm::clamp(
				thrusts[i],
				0.0f,
				g_drone.engines[i].maxThrust);

		g_engineCommands[i].engineId =
			g_drone.engines[i].id;

		g_engineCommands[i].thrust = thrust;
	}
}

static void updateTargetPosition(float dt) {
	glm::vec3 move(0);

	move.z += InputHandles::forward.getValue();
	move.x += InputHandles::left.getValue() - InputHandles::right.getValue();

	if (glm::length(move) > 0.0f)
		move = glm::normalize(move);

	move *= MOVE_SPEED;

	g_targetPosition += move * dt;

	if (InputHandles::up.getValue() == ButtonStateCpp::Pressed || InputHandles::up.getValue() == ButtonStateCpp::Down)
		g_targetPosition.y += ALTITUDE_SPEED * dt;

	if (InputHandles::down.getValue() == ButtonStateCpp::Pressed || InputHandles::down.getValue() == ButtonStateCpp::Down)
		g_targetPosition.y -= ALTITUDE_SPEED * dt;


	g_targetPosition.y = g_targetPosition.y < -10 ? -10 : g_targetPosition.y;
}

static glm::vec3 toVec(const float* data) {
	return glm::vec3{ data[0], data[1], data[2] };
}
static glm::quat toQuat(const float* data) {
	return glm::quat{ data[0], data[1], data[2], data[3] };
}

DRONE_API void getTargetPosition(float* outPosition) {
	outPosition[0] = g_targetPosition.x;
	outPosition[1] = g_targetPosition.y;
	outPosition[2] = g_targetPosition.z;
}

namespace PIDController {
std::optional<float*> getSetting(const std::string& name) noexcept {
	for (size_t i = 0; i < g_settings.count; i++) {
		if (g_settings.names[i] == name) {
			return g_settings.values[i];
		}
	}
	return {};
}

void addSettings(const char* name, float& value) {
	assert(!g_initialized && "Settings must be added before calling PIDController::setup");
	g_settingNames.push_back(name);
	g_settingValues.push_back(&value);
}

void setup(const char* dronePath, const UserInput* input) {
	g_drone = getDrone(dronePath);

	g_inputHandler.startUp(*input);

	g_settings = {
		.names = g_settingNames.data(),
		.values = g_settingValues.data(),
		.count = g_settingNames.size()
	};
}
void update(
	const DroneState* state,
	const float dt,
	CommandBuffer* outCommands) {

	assert(state && "The state is nullptr");
	assert(dt > 0 && "The dt is not grater than 0");
	assert(outCommands && "The out commands in nullptr");

	glm::vec3 position = toVec(state->position);
	glm::vec3 velocity = toVec(state->velocity);
	glm::quat orientation = toQuat(state->orientation);
	glm::vec3 angularVelocity = toVec(state->angularVelocity);

	if (!g_initialized) {
		g_targetPosition = position;
		g_initialized = true;
	}

	updateTargetPosition(dt);

	glm::vec3 positionError = g_targetPosition - position;

	glm::vec3 desiredVelocity = g_controller.positionPID.update(
		positionError,
		POS_KP,
		POS_KI,
		dt);

	desiredVelocity = clampMagnitude(desiredVelocity, MOVE_SPEED);
	glm::vec3 velocityError = desiredVelocity - velocity;

	g_controller.velocityPID.integral += velocityError * dt;
	glm::vec3 desiredAcceleration = VEL_KP * velocityError + VEL_KI * g_controller.velocityPID.integral - VEL_KD * velocity;

	desiredAcceleration += glm::vec3(0, GRAVITY, 0);
	float vertical = desiredAcceleration.y;
	desiredAcceleration = clampMagnitude(desiredAcceleration, 20.0f);
	desiredAcceleration.y = vertical;

	glm::vec3 up = glm::normalize(desiredAcceleration);
	glm::vec3 worldForward(0, 0, -1);
	glm::vec3 right = glm::normalize(glm::cross(worldForward, up));
	glm::vec3 forward = glm::normalize(glm::cross(up, right));

	glm::mat3 rotMatrix(
		right,
		up,
		-forward);

	glm::quat targetOrientation = glm::normalize(glm::quat_cast(rotMatrix));

	glm::quat qError = targetOrientation * glm::conjugate(orientation);

	if (qError.w < 0.0f)
		qError = -qError;

	glm::vec3 attitudeError = glm::axis(qError) * glm::angle(qError);

	glm::vec3 desiredAngularRate =
		g_controller.attitudePID.update(
			attitudeError,
			ATT_KP,
			ATT_KI,
			dt);

	glm::vec3 angularRateError = desiredAngularRate - angularVelocity;

	glm::vec3 desiredTorque = RATE_KP * angularRateError - RATE_KD * angularVelocity;
	glm::vec3 desiredForce = desiredAcceleration * g_drone.mass;

	solveSystem(desiredForce, desiredTorque);

	outCommands->commands = g_engineCommands.data();
	outCommands->count = ENGINE_COUNT;
}
}