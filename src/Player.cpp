#include "Player.hpp"

Player::Player() noexcept
	: _camera()
	, _drone()
{
	_camera.getPositionRef() = glm::vec3(0, 2, -5);
	_camera.updateViewMatrix();
}

Player::Player(std::filesystem::path folderPath)
	: _drone(folderPath)
	, _camera() {
	_camera.getPositionRef() = glm::vec3(0, 2, 0);
	_camera.updateViewMatrix();
}

void Player::SwapDrone(std::filesystem::path folderPath) {
	if (!_drone.has_value()) {
		_drone.emplace(folderPath);
		return;
	}
	DroneState state = _drone->getState();
	_drone.reset();
	_drone.emplace(folderPath, state);
}

void Player::releaseDrone() {
	_drone.reset();
}

vulkan::UniformBufferObject Player::getUBO() const noexcept {
	using vulkan::Camera;
	if (!_drone.has_value()) {
		return {
			.proj = _camera.getProjection(),
			.view = _camera.getView(),
			.cameraPos = glm::vec4(_camera.getPosition(), 0.0),
			.lightSource = glm::vec4()
		};
	}
	
	if (_camera.getState() == Camera::State::FreeCAM)
		return {
			.proj = _camera.getProjection(),
			.view = _camera.getView(),
			.cameraPos = glm::vec4(_camera.getPosition(), 0.0),
			.lightSource = glm::vec4()
		};

	glm::vec3 cameraPosition = _camera.getPosition() * glm::conjugate(_drone->getOrientation()) + _drone->getPosition();
	glm::quat cameraOrientation = glm::quatLookAt(glm::normalize(_camera.getPosition() * glm::conjugate(_drone->getOrientation())), glm::rotate(_drone->getOrientation(), { 0, -1, 0 }));
	return {
		.proj = _camera.getProjection(),
		.view = vulkan::getViewMatrix(cameraPosition, cameraOrientation),
		.cameraPos = glm::vec4(cameraPosition, 0.0),
		.lightSource = glm::vec4()
	};
}

void Player::update(bool updateCamera) {
	using vulkan::Camera;
	if (ImGui::IsKeyPressed(ImGuiKey_F))
		_camera.setState(Camera::State::FreeCAM);
	if (ImGui::IsKeyPressed(ImGuiKey_T))
		_camera.setState(Camera::State::Orbit);

	if (updateCamera)
		_camera.update();
	if (_drone.has_value())
		_drone->update();
}
