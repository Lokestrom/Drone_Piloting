#pragma once

namespace API {
#include "../API/DroneAPI.h"
}

enum class ButtonState : uint8_t {
	Pressed = API::ButtonState_Pressed,
	Down = API::ButtonState_Down,
	Released = API::ButtonState_Released,
	Up = API::ButtonState_Up
};

enum class Input {
	// Keyboard
	Key_W,
	Key_A,
	Key_S,
	Key_D,
	Key_Q,
	Key_E,
	Key_Shift,
	Key_Space,

	// Mouse
	Mouse_LeftButton,
	Mouse_RightButton,
	Mouse_MiddleButton,

	// Gamepad
	Gamepad_ButtonA,
	Gamepad_ButtonB,
	Gamepad_ButtonX,
	Gamepad_ButtonY,
	Gamepad_LeftBumper,
	Gamepad_RightBumper,
	Gamepad_Back,
	Gamepad_Start,
	Gamepad_Guide,
	Gamepad_LeftThumb,
	Gamepad_RightThumb,
};

struct Bind {
	Input input;
	ButtonState value;
};