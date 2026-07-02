#include "Player.hpp"

#include "App.hpp"
#include "console.hpp"
#include "SettingNames.hpp"

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

bool Player::SwapDrone(std::filesystem::path folderPath) noexcept {
	try {
		if (!_drone.has_value()) {
			_camera.setState(vulkan::Camera::State::Orbit);
			_drone.emplace();
			if (!_drone->load(folderPath)) {
				_camera.setState(vulkan::Camera::State::FreeCAM);
				_drone.reset();
				return false;
			}
			return true;
		}
		API::DroneState state = _drone->getState();
		_drone.reset();
		_drone.emplace();
		if (!_drone->load(folderPath, state)) {
			_camera.setState(vulkan::Camera::State::FreeCAM);
			_drone.reset();
			return false;
		}
		return true;
	}
	catch (std::exception& e) {
		_camera.setState(vulkan::Camera::State::FreeCAM);
		_drone.reset();
		Console::log(Console::Log::Type::error, std::string("Hit an exception when trying to load drone: ") + e.what());
		return false;
	}
}

void Player::releaseDrone() {
	_camera.setState(vulkan::Camera::State::FreeCAM);
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

void Player::update(bool active, bool updateCamera) {
	using vulkan::Camera;
	if (ImGui::IsKeyPressed(_freeCamera.get()))
		_camera.setState(Camera::State::FreeCAM);
	if (ImGui::IsKeyPressed(_orbitCamera.get()))
		_camera.setState(Camera::State::Orbit);

	if (updateCamera)
		_camera.update();
	if (_drone.has_value())
		_drone->update(active);
}
