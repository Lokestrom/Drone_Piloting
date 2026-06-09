#pragma once

#include <filesystem>
#include <optional>

#include <glm/glm.hpp>

#include "rendering/gameObject.hpp"
namespace API {
#include "API/DroneAPI.h"
}
#include "structures/sharedLib.hpp"
#include "Input/InputEventHandler.hpp"
#include "Settings.hpp"

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
	API::UpdateFn update;

	API::GetTargetPositionFn getTargetPosition;
	API::GetSettingsFn getSettings;
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
	Drone(std::filesystem::path folderPath, API::DroneState state);
	~Drone() noexcept;

	void update();

	[[nodiscard]]
	glm::vec3& getPosition() noexcept;
	[[nodiscard]]
	const glm::vec3& getPosition() const noexcept;

	[[nodiscard]]
	glm::quat& getOrientation() noexcept;
	[[nodiscard]]
	const glm::quat& getOrientation() const noexcept;

	[[nodiscard]]
	glm::vec3& getVelocity() noexcept { return _velocity; }
	[[nodiscard]]
	glm::vec3& getRotationalVelocity() noexcept { return _angularMomentum; }

	[[nodiscard]]
	API::DroneState getState() const noexcept;

	[[nodiscard]]
	bool hasSettings() const noexcept { return _plugin.getSettings != nullptr; }
	[[nodiscard]]
	API::SettingsBuffer* getSettings() const noexcept { return _plugin.getSettings(); }
	
	[[nodiscard]]
	bool hasTarget() const noexcept { return _plugin.getTargetPosition != nullptr; }
	[[nodiscard]]
	glm::vec3 getTarget() const noexcept { 
		glm::vec3 target;
		_plugin.getTargetPosition(&target.x);
		return target; 
	}

	settings::Settings _settings;
private:
	void load(std::filesystem::path folderPath);

	vulkan::GameObject& getObject() const noexcept;

	void populateInput() noexcept;

private:
	vulkan::ID objectID;

	glm::mat3 _invInertia_B;

	glm::vec3 _velocity;
	glm::vec3 _angularMomentum;
	float _mass;

	std::unordered_map<uint64_t, Engines> _engines;
	DronePlugin _plugin;

	API::UserInput _input;
	std::vector<std::string> _inputNames;
	std::vector<const char*> _inputNamePtrs;
	std::vector<API::InputType> _inputType;
	std::vector<ButtonState> _inputButtonStates;
};