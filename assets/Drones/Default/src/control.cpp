#include "D:/code/codeProjects/Drone_piloting/src/DroneAPI.h"

DRONE_API void update(
	const UserInput* input,
	const DroneState* state,
	CommandBuffer* outCommands) {
	const float rotorForce = 20.0f;
	float thrust = 0.0f;

	if (input->keyW)
		thrust += rotorForce;

	if (input->keyS)
		thrust -= rotorForce;

	static EngineCommand commands[6];

	commands[0].engineId = 0;
	commands[0].thrust = thrust;
	commands[1].engineId = 1;
	commands[1].thrust = thrust;
	commands[2].engineId = 2;
	commands[2].thrust = thrust;
	commands[3].engineId = 3;
	commands[3].thrust = thrust;
	commands[4].engineId = 4;
	commands[4].thrust = thrust;
	commands[5].engineId = 5;
	commands[5].thrust = thrust;

	outCommands->commands = commands;
	outCommands->count = 6;
}