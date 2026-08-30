#pragma once

#include "Drone.hpp"
#include <renderer/Camera.hpp>
#include <renderer/Renderer.hpp>

#include <optional>

class Player {
public:
	Player(const std::string& name) noexcept;

	Player(Player&) = delete;
	Player& operator=(Player&) = delete;

	Player(Player&&) noexcept = default;
	Player& operator=(Player&&) noexcept = delete;

	void replaceDrone(Drone&& replacement) noexcept;
	void releaseDrone() noexcept;

	[[nodiscard]]
	renderer::UniformBufferObject getUBO() const noexcept;

	void update(bool active, bool updateCamera);

	[[nodiscard]]
	renderer::Camera& getCamera() noexcept { return _camera; }
	[[nodiscard]]
	std::optional<Drone>& getDrone() noexcept { return _drone; }

	[[nodiscard]]
	const std::string& name() const noexcept { return _name; }
	void setName(const std::string& newName) { _name = newName; }

private:
	enum class CameraMode {
		Free,
		Orbit
	};

	void updateCameraProjection() noexcept;
	void updateFreeCamera();
	void updateOrbitCamera();

	std::string _name;
	std::optional<Drone> _drone;
	renderer::Camera _camera;
	CameraMode _cameraMode = CameraMode::Free;
	double _orbitRadius = 1.0;
	double _orbitYaw = 0.0;
	double _orbitPitch = 0.0;
	settings::ValueHandle<ImGuiKey> _freeCamera;
	settings::ValueHandle<ImGuiKey> _orbitCamera;
};
