// SPDX-License-Identifier: MIT

/********************************************************************
 * The MIT License (MIT)											*
 * Copyright © 2026 Loke Strøm Haugsnes <Lokesh@live.no>			*
 *																	*
 * Permission is hereby granted, free of charge, to any				*
 * person obtaining a copy of this software and associated			*
 * documentation files (the “Software”), to deal in the				*
 * Software without restriction, including without					*
 * limitation the rights to use, copy, modify, merge,				*
 * publish, distribute, sublicense, and/or sell copies of			*
 * the Software, and to permit persons to whom the Software			*
 * is furnished to do so, subject to the following					*
 * conditions:														*
 *																	*
 * The above copyright notice and this permission notice 			*
 * shall be included in all copies or substantial portions 			*
 * of the Software.													*
 * 																	*
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY		* 
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 			*
 * THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 				*
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT 				*
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY			*
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF		*
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 			*
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 		*
 * IN THE SOFTWARE.													*
 ********************************************************************/

#pragma once

#ifdef _WIN32
#define DRONE_API extern "C" __declspec(dllexport)
#else
#define DRONE_API extern "C"
#endif

#include <cstdint>

// TODO: create C++ wrappers in .hpp(GPLv3 license)

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
	uint64_t engineId;
	float thrust;
};

struct CommandBuffer {
	EngineCommand* commands;
	uint64_t count;
};

struct SettingsBuffer {
	const char** names;
	float** values;
	uint64_t count;
};

// Function pointer type
typedef void (*UpdateFn)(
	const UserInput* input,
	const DroneState* state,
	const float dt,
	CommandBuffer* outCommands);

typedef void (*SetupFn)(const char* dronePath);

typedef void (*GetTargetPositionFn)(float* outPosition);
typedef SettingsBuffer* (*GetSettingsFn)();