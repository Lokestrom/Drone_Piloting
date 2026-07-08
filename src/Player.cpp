#include "Player.hpp"

#include "App.hpp"
#include "SettingNames.hpp"

#include <utility>

Player::Player(const std::string& name) noexcept
	: _name(name)
	, _drone()
	, _camera()
	, _freeCamera(App::settings.get(settingNames::categories::keyBindings)
		.getSubCategory(settingNames::categories::camera)
		.get<ImGuiKey>(settingNames::cameraKeys::freeCamera)
		.getHandle())
	, _orbitCamera(App::settings.get(settingNames::categories::keyBindings)
		.getSubCategory(settingNames::categories::camera)
		.get<ImGuiKey>(settingNames::cameraKeys::orbitCamera)
		.getHandle()) {
	_camera.setState(vulkan::Camera::State::FreeCAM);
	_camera.getPositionRef() = glm::vec3(0, 2, -5);
	_camera.updateViewMatrix();
}

void Player::replaceDrone(Drone&& replacement) noexcept {
	_drone.emplace(std::move(replacement));
	_camera.setState(vulkan::Camera::State::Orbit);
}

void Player::releaseDrone() noexcept {
	_camera.setState(vulkan::Camera::State::FreeCAM);
	_drone.reset();
}

vulkan::UniformBufferObject Player::getUBO() const noexcept {
	using vulkan::Camera;
	if (!_drone) {
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

void Player::update(bool active, bool updateCamera) {
	using vulkan::Camera;
	if (ImGui::IsKeyPressed(_freeCamera.get())) [[unlikely]]
		_camera.setState(Camera::State::FreeCAM);
	if (ImGui::IsKeyPressed(_orbitCamera.get())) [[unlikely]]
		_camera.setState(Camera::State::Orbit);

	if (updateCamera)
		_camera.update();
	if (_drone)
		_drone->update(active);
}
