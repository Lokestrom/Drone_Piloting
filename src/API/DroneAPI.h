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
	Axis1Way, // [0, 1] or button 0 or 1
	Axis2Way  // [-1, 1] or two buttons -1 or 0 or 1
};

/*
size is the total amount of inputs.
names are the names that can be used to find the inputs, it is valid only in the startup function
The types is InputType if it is Button then the value is in buttonPressed else it is axisValues
The values in the types, buttonPressed, and axisValues arrays are valid for the duration of your scripts lifetime
buttonPressed contains the state of the buttons and axisValues the state of the axis inputs
The values in buttonPressed and axisValues are updated each update
The values in types are never updated so it is probably never used outside of the startup function
*/

struct UserInput {
	uint64_t size;
	const char** names;
	InputType* types;
	ButtonState* buttonPressed;
	float* axisValues;
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
	const DroneState* state,
	const float dt,
	const bool active,
	CommandBuffer* outCommands);

typedef void (*SetupFn)(const char* dronePath, const UserInput* input);

typedef void (*GetTargetPositionFn)(float* outPosition);
typedef SettingsBuffer* (*GetSettingsFn)();
