#include "D:/code/codeProjects/Drone_piloting/src/DroneAPI.h"

DRONE_API void update(
	const UserInput* input,
	const DroneState* state,
	CommandBuffer* outCommands) {
	const float rotorForce = 10.0f;

	static EngineCommand commands[4];

	commands[0].engineId = 0;
	commands[0].thrust = 0.0f;
	commands[1].engineId = 1;
	commands[1].thrust = 0.0f;
	commands[2].engineId = 2;
	commands[2].thrust = 0.0f;
	commands[3].engineId = 3;
	commands[3].thrust = 0.0f;

	if (input->keySpace) {
		commands[0].thrust = rotorForce;
		commands[1].thrust = rotorForce;
		commands[2].thrust = rotorForce;
		commands[3].thrust = rotorForce;
	}

	if(input->keyW) {
		commands[1].thrust += rotorForce;
		commands[3].thrust += rotorForce;
	}
	if (input->keyS) {
		commands[0].thrust += rotorForce;
		commands[2].thrust += rotorForce;
	}

	if (input->keyD) {
		commands[0].thrust += rotorForce;
		commands[1].thrust += rotorForce;
	}
	if (input->keyA) {
		commands[2].thrust += rotorForce;
		commands[3].thrust += rotorForce;
	}

	outCommands->commands = commands;
	outCommands->count = 4;
}