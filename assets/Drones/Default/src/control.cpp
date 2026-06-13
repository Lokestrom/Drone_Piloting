#define ENGINE_COUNT 6

#include "../../../../src/API/helpers/PIDcontroler.hpp"

DRONE_API void setup(const char* dronePath, const UserInput* input) {
	PIDController::setup(dronePath, input);
	float& moveSpeed = *PIDController::getSetting("MOVE_SPEED").value();
	moveSpeed = 10.;
	float& altitudeSpeed = *PIDController::getSetting("ALTITUDE_SPEED").value();
	altitudeSpeed = 5.;
	float& posKP = *PIDController::getSetting("POS_KP").value();
	posKP = 1.;
	float& posKI = *PIDController::getSetting("POS_KI").value();
	posKI = 0.1;
	float& velKP = *PIDController::getSetting("VEL_KP").value();
	velKP = 1.;
	float& velKI = *PIDController::getSetting("VEL_KI").value();
	velKI = 0.2;
	float& velKD = *PIDController::getSetting("VEL_KD").value();
	velKD = 0.5;
	float& attKP = *PIDController::getSetting("ATT_KP").value();
	attKP = 5.;
	float& attKI = *PIDController::getSetting("ATT_KI").value();
	attKI = 0.1;
	float& rateKP = *PIDController::getSetting("RATE_KP").value();
	rateKP = 2.;
	float& rateKD = *PIDController::getSetting("RATE_KD").value();
	rateKD = 0.5;
}

DRONE_API void update(
	const DroneState* state,
	const float dt,
	CommandBuffer* outCommands) {
	PIDController::update(state, dt, outCommands);
}