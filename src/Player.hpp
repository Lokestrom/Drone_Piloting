#pragma once

#include "Drone.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Renderer.hpp"

#include <optional>
#include <filesystem>

class Player {
public:
	Player(const std::string& name) noexcept;

	Player(Player&) = delete;
	Player& operator=(Player&) = delete;

	Player(Player&&) noexcept = default;
	Player& operator=(Player&&) noexcept = delete;

	[[nodiscard]]
	bool SwapDrone(std::filesystem::path folderPath) noexcept;
	void releaseDrone();

	[[nodiscard]]
	vulkan::UniformBufferObject getUBO() const noexcept;

	void update(bool active, bool updateCamera);

	[[nodiscard]]
	vulkan::Camera& getCamera() noexcept { return _camera; }
	[[nodiscard]]
	std::optional<Drone>& getDrone() noexcept { return _drone; }

	[[nodiscard]]
	const std::string& name() const noexcept { return _name; }
	void setName(const std::string& newName) { _name = newName; }

private:
	std::string _name;
	std::optional<Drone> _drone;
	vulkan::Camera _camera;
};