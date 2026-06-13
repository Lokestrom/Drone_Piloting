#define ENGINE_COUNT 4

#include "../../../../src/API/helpers/PIDcontroler.hpp"

DRONE_API void setup(const char* dronePath, const UserInput* input) {
	PIDController::setup(dronePath, input);
}

DRONE_API void update(
	const DroneState* state,
	const float dt,
	CommandBuffer* outCommands) {
	PIDController::update(state, dt, outCommands);
}