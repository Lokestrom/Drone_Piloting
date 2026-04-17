#pragma once

#ifdef _WIN32
#define DRONE_API extern "C" __declspec(dllexport)
#else
#define DRONE_API extern "C"
#endif

struct UserInput {
	bool keyW;
	bool keyS;
	bool keyA;
	bool keyD;
	bool keySpace;
	bool keyShift;
};

struct DroneState {
	float position[3];
	float velocity[3];
	float orientation[4];
	float angularVelocity[3];
};

struct EngineCommand {
	size_t engineId;
	float thrust;
};

struct CommandBuffer {
	EngineCommand* commands;
	int count;
};

// Function pointer type
typedef void (*UpdateFn)(
	const UserInput* input,
	const DroneState* state,
	CommandBuffer* outCommands);