#pragma once

#include "rendering/gameObject.hpp"

#include <filesystem>

class Drone {
public:

	Drone(std::filesystem::path folderPath);
	~Drone() noexcept;

	void update();

	glm::vec3& getPosition() noexcept;
	const glm::vec3& getPosition() const noexcept;

	glm::quat& getOrientation() noexcept;
	const glm::quat& getOrientation() const noexcept;

	glm::vec3& getVelocity() noexcept { return _velocity; }

private:
	vulkan::ID objectID;

	glm::vec3 _velocity;
	glm::vec3 _rotationalVelocity;
	float _mass;
};