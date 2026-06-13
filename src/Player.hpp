#pragma once

#include "Drone.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Renderer.hpp"

#include <optional>
#include <filesystem>

class Player {
public:
	Player() noexcept;
	Player(std::filesystem::path folderPath);

	Player(Player&) = delete;
	Player& operator=(Player&) = delete;

	Player(Player&&) noexcept = default;
	Player& operator=(Player&&) noexcept = delete;

	void SwapDrone(std::filesystem::path folderPath);
	void releaseDrone();

	[[nodiscard]]
	vulkan::UniformBufferObject getUBO() const noexcept;

	void update(bool updateCamera);

	[[nodiscard]]
	vulkan::Camera& getCamera() noexcept { return _camera; }
	[[nodiscard]]
	std::optional<Drone>& getDrone() noexcept { return _drone; }

private:
	std::optional<Drone> _drone;
	vulkan::Camera _camera;
};