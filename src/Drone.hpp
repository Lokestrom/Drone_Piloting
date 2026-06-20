#pragma once

#include <filesystem>
#include <optional>

#include <glm/glm.hpp>
#include "importJSONData.hpp"

#include "rendering/gameObject.hpp"
namespace API {
#include "API/DroneAPI.h"
}
#include "structures/sharedLib.hpp"
#include "Input/InputEventHandler.hpp"
#include "Settings.hpp"

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
	
	Drone() noexcept = default;
	
	Drone(Drone&) = delete;
	Drone& operator=(Drone&) = delete;

	Drone(Drone&&) noexcept;
	Drone& operator=(Drone&&) noexcept;
	
	~Drone() noexcept;

	[[nodiscard]]
	bool load(const std::filesystem::path& folderPath);
	[[nodiscard]]
	bool load(const std::filesystem::path& folderPath, const API::DroneState& state);

	void update(bool active);

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
	[[nodiscard]]
	bool _load(const std::filesystem::path& folderPath);

	[[nodiscard]]
	bool verifyFolder(const std::filesystem::path& folderPath) const;
	[[nodiscard]]
	bool verifyConfigFile(const Json& jsonData, const std::filesystem::path& folderPath) const;
	[[nodiscard]]
	bool verifyPlugin(const SharedLib& pluginLib) const;

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
	std::vector<API::InputType> _inputType;
	std::vector<ButtonState> _inputButtonStates;
	std::vector<float> _inputAxisStates;
};