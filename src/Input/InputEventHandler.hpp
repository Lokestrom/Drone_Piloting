#pragma once

#include <array>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "keybinds.hpp"
namespace API {
#include "../API/DroneAPI.h"
}

class InputEventHandler {
public:
	static inline double mouseScrollWheel;
	
	static void reset() noexcept { mouseScrollWheel = 0; }

	static inline glm::vec<2, double> lastMousePos;
	static inline glm::vec<2, double> mouseDelta;
};

enum class Joystick : int {
	nr1 = GLFW_JOYSTICK_1,
	nr2 = GLFW_JOYSTICK_2,
	nr3 = GLFW_JOYSTICK_3,
	nr4 = GLFW_JOYSTICK_4,
	nr5 = GLFW_JOYSTICK_5,
	nr6 = GLFW_JOYSTICK_6,
	nr7 = GLFW_JOYSTICK_7,
	nr8 = GLFW_JOYSTICK_8,
	nr9 = GLFW_JOYSTICK_9,
	nr10 = GLFW_JOYSTICK_10,
	nr11 = GLFW_JOYSTICK_11,
	nr12 = GLFW_JOYSTICK_12,
	nr13 = GLFW_JOYSTICK_13,
	nr14 = GLFW_JOYSTICK_14,
	nr15 = GLFW_JOYSTICK_15,
	nr16 = GLFW_JOYSTICK_16
};

enum class GamepadButton : int {
	A = GLFW_GAMEPAD_BUTTON_A,
	B = GLFW_GAMEPAD_BUTTON_B,
	X = GLFW_GAMEPAD_BUTTON_X,
	Y = GLFW_GAMEPAD_BUTTON_Y,

	Cross = GLFW_GAMEPAD_BUTTON_CROSS,
	Circle = GLFW_GAMEPAD_BUTTON_CIRCLE,
	Square = GLFW_GAMEPAD_BUTTON_SQUARE,
	Triangle = GLFW_GAMEPAD_BUTTON_TRIANGLE,

	LeftBumper = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
	RightBumper = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,

	Back = GLFW_GAMEPAD_BUTTON_BACK,
	Start = GLFW_GAMEPAD_BUTTON_START,
	Guide = GLFW_GAMEPAD_BUTTON_GUIDE,

	LeftThumb = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
	RightThumb = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,

	DPadUp = GLFW_GAMEPAD_BUTTON_DPAD_UP,
	DPadRight = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
	DPadDown = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
	DPadLeft = GLFW_GAMEPAD_BUTTON_DPAD_LEFT
};

enum class GamepadAxis : int {
	LeftX = GLFW_GAMEPAD_AXIS_LEFT_X,
	LeftY = GLFW_GAMEPAD_AXIS_LEFT_Y,

	RightX = GLFW_GAMEPAD_AXIS_RIGHT_X,
	RightY = GLFW_GAMEPAD_AXIS_RIGHT_Y,

	LeftTrigger = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
	RightTrigger = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER
};


// https://www.glfw.org/docs/latest/input_guide.html#gamepad
class GamePad {
public:
	void update() {
		previousButtons = currentButtons;

		if (!glfwJoystickIsGamepad(static_cast<int>(Joystick::nr1))) {
			reset();
			return;
		}

		GLFWgamepadstate state;
		if (!glfwGetGamepadState(static_cast<int>(Joystick::nr1), &state)) {
			reset();
			return;
		}

		connected = true;


		for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
			currentButtons[i] = state.buttons[i] == GLFW_PRESS;
		}

		for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; ++i) {
			axes[i] = state.axes[i];
		}
	}

	[[nodiscard]]
	bool isConnected() const noexcept {
		return connected;
	}

	[[nodiscard]]
	ButtonState button(GamepadButton button) const noexcept {
		int buttonIndex = static_cast<int>(button);
		const bool current = currentButtons[buttonIndex];
		const bool previous = previousButtons[buttonIndex];

		if (current) {
			return previous
					   ? ButtonState::Down
					   : ButtonState::Pressed;
		}

		return previous
				   ? ButtonState::Released
				   : ButtonState::Up;
	}

	[[nodiscard]]
	float axis(GamepadAxis axis) const noexcept {
		return axes[static_cast<int>(axis)];
	}

private:
	void reset() noexcept {
		connected = false;
		currentButtons.fill(false);
		previousButtons.fill(false);
		axes.fill(0.0f);
	}

private:
	bool connected = false;

	std::array<bool, GLFW_GAMEPAD_BUTTON_LAST + 1> currentButtons{};
	std::array<bool, GLFW_GAMEPAD_BUTTON_LAST + 1> previousButtons{};

	std::array<float, GLFW_GAMEPAD_AXIS_LAST + 1> axes{};
};