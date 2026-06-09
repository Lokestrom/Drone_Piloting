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

enum ButtonState : uint8_t {
	ButtonState_Pressed,
	ButtonState_Down,
	ButtonState_Released,
	ButtonState_Up
};

enum InputType : uint8_t {
	Button,
	Axis1Way, // [0, 1] or button (down = 1, up = 0)
	Axis2Way  // [-1, 1] or two buttons (negative = -1, neutral = 0, positive = 1)
};

/*
Usage:
	The names are set by the user at startup and are guaranteed to be the same size from load to unload
	the inputs have no guaranteed order, but they are guaranteed to be in the same order unless the changed flag is set to true, 
	then the order can change and the user should check the names to know which input is which. 
	An input may also change type, this is to allow for controllers but not hinder keyboard users
	This is not a requirement to actually implement this but it will allow your drone to be used with a wider variety of users.
	axis values are guaranteed to be in the range [-1, 1]
	Also a pointer to a value is valid until the changed flag is set
*/
struct UserInput {
	const char** names;
	InputType* types;
	ButtonState* buttonPressed;
	uint64_t buttonCount;
	float* axisValues;
	uint64_t axisCount;

	bool changed;
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
