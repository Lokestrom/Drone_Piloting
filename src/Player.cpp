#include "Player.hpp"

Player::Player() noexcept
	: _camera()
	, _drone()
, freeCAM(false) {
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
	_drone.reset();
	_drone.emplace(folderPath);
}

void Player::releaseDrone() {
	_drone.reset();
}

vulkan::UniformBufferObject Player::getUBO() const noexcept {
	if (!_drone.has_value() || freeCAM) {
		return {
			.proj = _camera.getProjection(),
			.view = _camera.getView(),
			.cameraPos = glm::vec4(_camera.getPosition(), 0.0),
			.lightSource = glm::vec4()
		};
	}

	glm::vec3 cameraPosition = _camera.getPosition() * glm::conjugate(_drone->getOrientation()) + _drone->getPosition();
	glm::quat cameraOrientation = glm::quatLookAt(glm::normalize(_camera.getPosition() * glm::conjugate(_drone->getOrientation())), { 0, -1, 0 });
	
	return {
		.proj = _camera.getProjection(),
		.view = getViewMatrix(cameraPosition, cameraOrientation),
		.cameraPos = glm::vec4(_camera.getPosition(), 0.0),
		.lightSource = glm::vec4()
	};
}

void Player::update() {
	if (ImGui::IsKeyPressed(ImGuiKey_R))
		freeCAM = !freeCAM;
	if (ImGui::IsKeyPressed(ImGuiKey_C))
		_camera.toggleMovementUpdating(!_camera.movementEnabled());

	_camera.update();
	if (!_camera.movementEnabled() && !freeCAM)
		_drone->update();
}
