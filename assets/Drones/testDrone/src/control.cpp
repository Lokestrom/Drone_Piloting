#define ENGINE_COUNT 4

#include "../../../../src/API/helpers/PIDcontroler.hpp"

DRONE_API void setup(const char* dronePath) {
	PIDController::setup(dronePath);
}

DRONE_API void update(
	const UserInput* input,
	const DroneState* state,
	const float dt,
	CommandBuffer* outCommands) {
	PIDController::update(input, state, dt, outCommands);
}