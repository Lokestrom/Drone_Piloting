#pragma once

#include <filesystem>
#include <optional>

#include <glm/glm.hpp>

#include "rendering/gameObject.hpp"
#include "DroneAPI.h"
#include "sharedLib.hpp"

struct Error {
public:		
	Error() noexcept = default;
	[[nodiscard]] bool hasError() const noexcept { return message.has_value(); }
	[[nodiscard]] const std::string& getMessage() const noexcept { return message.value(); }

	void operator+=(const std::string& msg) {
		if (message.has_value()) {
			message.value() += msg;
		}
		else {
			message = msg;
		}
	}

private:
	std::optional<std::string> message;

};

struct DronePlugin {
	SharedLib lib;
	UpdateFn update;
};

class Drone {
public:
	struct Engines {
		unsigned int id;
		float maxThrust;
		glm::vec3 position;
		glm::vec3 direction;
	};

	Drone(std::filesystem::path folderPath);
	Drone(std::filesystem::path folderPath, DroneState state);
	~Drone() noexcept;

	void update();

	glm::vec3& getPosition() noexcept;
	const glm::vec3& getPosition() const noexcept;

	glm::quat& getOrientation() noexcept;
	const glm::quat& getOrientation() const noexcept;

	glm::vec3& getVelocity() noexcept { return _velocity; }
	glm::vec3& getRotationalVelocity() noexcept { return _angularMomentum; }

	DroneState getState() const noexcept;

private:
	void load(std::filesystem::path folderPath);

	vulkan::GameObject& getObject() const noexcept;

private:
	vulkan::ID objectID;

	glm::mat3 _invInertia_B;

	glm::vec3 _velocity;
	glm::vec3 _angularMomentum;
	float _mass;

	std::unordered_map<unsigned int, Engines> _engines;
	DronePlugin _plugin;
};