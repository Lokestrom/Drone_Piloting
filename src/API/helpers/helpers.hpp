#include "../DroneAPI.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <span>
#include <vector>


struct DroneEngine {
	size_t id;
	glm::vec3 position;
	glm::vec3 direction;
	float maxThrust;
};

struct Drone {
	float mass;
	glm::mat3 inertiaTensor;
	std::vector<DroneEngine> engines;
};

Drone getDrone(const char* dronePath);

enum class ButtonStateCpp : uint8_t {
	Pressed = ButtonState_Pressed,
	Down = ButtonState_Down,
	Released = ButtonState_Released,
	Up = ButtonState_Up
};

class UserInputHandler {
public:
	class Handle {
		friend UserInputHandler;
	public:
		Handle() = delete;
		Handle(std::string_view name)
			: name(name) {
		}

		const std::string_view name;
	protected:
		void* valuePtr = nullptr;
	};

	// gives values in the range [0, 1] and can be substituted with a button (down = 1, up = 0)
	class HandleAxis1 : public Handle {
	public:
		HandleAxis1(std::string_view name)
			: Handle(name) {}
		float getValue() const noexcept;
	};

	// gives values in the range [-1, 1] and can be substituted with two buttons (negative = -1, neutral = 0, positive = 1)
	class HandleAxis2 : public Handle {
	public:
		HandleAxis2(std::string_view name)
			: Handle(name) {}
		float getValue() const noexcept;
	};
	class HandleButton : public Handle {
	public:
		HandleButton(std::string_view name)
			: Handle(name) {}
		ButtonStateCpp getValue() const noexcept;
	};

	// should be called on startup to check if the program has handles for all the inputs
	static void startUp(const UserInput& input, std::span<Handle* const> handles) noexcept;
};
