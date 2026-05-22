#pragma once

#include <filesystem>
#include <optional>

#include <glm/glm.hpp>

#include "rendering/gameObject.hpp"
#include "API/DroneAPI.h"
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

	GetTargetPositionFn getTargetPosition;
	GetSettingsFn getSettings;
};

// have to separate it into 2 where one is just pure physics

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

	bool hasSettings() const noexcept { return _plugin.getSettings != nullptr; }
	SettingsBuffer* getSettings() const noexcept { return _plugin.getSettings(); }
	
	bool hasTarget() const noexcept { return _plugin.getTargetPosition != nullptr; }
	glm::vec3 getTarget() const noexcept { 
		glm::vec3 target;
		_plugin.getTargetPosition(&target.x);
		return target; 
	}

private:
	void load(std::filesystem::path folderPath);

	vulkan::GameObject& getObject() const noexcept;

private:
	vulkan::ID objectID;

	glm::mat3 _invInertia_B;

	glm::vec3 _velocity;
	glm::vec3 _angularMomentum;
	float _mass;

	std::unordered_map<uint64_t, Engines> _engines;
	DronePlugin _plugin;
};