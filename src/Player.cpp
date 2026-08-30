#include "Player.hpp"

#include "App.hpp"
#include "Input/InputEventHandler.hpp"
#include "SettingNames.hpp"

#include <algorithm>
#include <cmath>
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
	_camera.setPosition(glm::vec3(0, 2, -5));
	updateCameraProjection();
}

void Player::replaceDrone(Drone&& replacement) noexcept {
	_drone.emplace(std::move(replacement));
	_cameraMode = CameraMode::Orbit;
}

void Player::releaseDrone() noexcept {
	_cameraMode = CameraMode::Free;
	_drone.reset();
}

renderer::UniformBufferObject Player::getUBO() const noexcept {
	if (!_drone) {
		return {
			.proj = _camera.getProjection(),
			.view = _camera.getView(),
			.cameraPos = glm::vec4(_camera.getPosition(), 0.0),
			.lightSource = glm::vec4()
		};
	}
	
	if (_cameraMode == CameraMode::Free)
		return {
			.proj = _camera.getProjection(),
			.view = _camera.getView(),
			.cameraPos = glm::vec4(_camera.getPosition(), 0.0),
			.lightSource = glm::vec4()
		};

	const glm::vec3 cameraPosition = _camera.getPosition() * glm::conjugate(_drone->getOrientation()) + _drone->getPosition();
	const glm::quat cameraOrientation = glm::quatLookAt(
		glm::normalize(_camera.getPosition() * glm::conjugate(_drone->getOrientation())),
		glm::rotate(_drone->getOrientation(), glm::vec3(0.0f, -1.0f, 0.0f)));
	return {
		.proj = _camera.getProjection(),
		.view = renderer::getViewMatrix(cameraPosition, cameraOrientation),
		.cameraPos = glm::vec4(cameraPosition, 0.0),
		.lightSource = glm::vec4()
	};
}

void Player::update(bool active, bool updateCamera) {
	if (ImGui::IsKeyPressed(_freeCamera.get())) [[unlikely]]
		_cameraMode = CameraMode::Free;
	if (ImGui::IsKeyPressed(_orbitCamera.get())) [[unlikely]]
		_cameraMode = CameraMode::Orbit;

	updateCameraProjection();
	if (updateCamera) {
		if (_cameraMode == CameraMode::Free) {
			updateFreeCamera();
		}
		else {
			updateOrbitCamera();
		}
	}
	if (_drone)
		_drone->update(active);
}

void Player::updateCameraProjection() noexcept {
	if (App::width <= 0 || App::height <= 0) {
		return;
	}
	auto& cameraSettings = App::settings.get(settingNames::categories::camera);
	const float fieldOfView = glm::radians(static_cast<float>(cameraSettings.get<double>(settingNames::camera::fieldOfView)));
	_camera.setPerspective(fieldOfView, static_cast<float>(App::width) / static_cast<float>(App::height), 0.01f, 10000.0f);
}

void Player::updateFreeCamera() {
	auto& settings = App::settings.get(settingNames::categories::camera);
	auto& keys = App::settings.get(settingNames::categories::keyBindings)
		.getSubCategory(settingNames::categories::camera);
	const auto isDown = [&keys](const char* name) {
		return ImGui::IsKeyDown(keys.get<ImGuiKey>(name));
	};

	const glm::quat orientation = _camera.getOrientation();
	const glm::vec3 forward = glm::rotate(orientation, glm::vec3(0.0f, 0.0f, 1.0f));
	const glm::vec3 right = glm::rotate(orientation, glm::vec3(-1.0f, 0.0f, 0.0f));
	const glm::vec3 up = glm::rotate(orientation, glm::vec3(0.0f, -1.0f, 0.0f));
	glm::vec3 movement{};
	movement += static_cast<float>(isDown(settingNames::cameraKeys::moveForward)) * forward;
	movement -= static_cast<float>(isDown(settingNames::cameraKeys::moveBackwards)) * forward;
	movement += static_cast<float>(isDown(settingNames::cameraKeys::moveLeft)) * right;
	movement -= static_cast<float>(isDown(settingNames::cameraKeys::moveRight)) * right;
	movement += static_cast<float>(isDown(settingNames::cameraKeys::moveUp)) * up;
	movement -= static_cast<float>(isDown(settingNames::cameraKeys::moveDown)) * up;
	if (glm::length2(movement) > 0.0f) {
		const float moveSpeed = static_cast<float>(settings.get<double>(settingNames::camera::moveSpeed));
		_camera.translate(glm::normalize(movement) * moveSpeed * static_cast<float>(App::getDeltaTime()));
	}

	const glm::vec3 pitchAxis(1.0f, 0.0f, 0.0f);
	const glm::vec3 yawAxis(0.0f, 1.0f, 0.0f);
	const glm::vec3 rollAxis(0.0f, 0.0f, -1.0f);
	glm::vec3 rotation{};
	rotation += static_cast<float>(isDown(settingNames::cameraKeys::rotateRight)) * yawAxis;
	rotation -= static_cast<float>(isDown(settingNames::cameraKeys::rotateLeft)) * yawAxis;
	rotation += static_cast<float>(isDown(settingNames::cameraKeys::rotateUp)) * pitchAxis;
	rotation -= static_cast<float>(isDown(settingNames::cameraKeys::rotateDown)) * pitchAxis;
	rotation += static_cast<float>(isDown(settingNames::cameraKeys::rollLeft)) * rollAxis;
	rotation -= static_cast<float>(isDown(settingNames::cameraKeys::rollRight)) * rollAxis;
	const float keyboardSpeed = static_cast<float>(settings.get<double>(settingNames::camera::keyboardRotationSpeed));
	const float mouseSensitivity = static_cast<float>(settings.get<double>(settingNames::camera::mouseSensitivity));
	rotation *= keyboardSpeed;
	rotation -= pitchAxis * mouseSensitivity * static_cast<float>(InputEventHandler::mouseDelta.y) * 10.0f;
	rotation += yawAxis * mouseSensitivity * static_cast<float>(InputEventHandler::mouseDelta.x) * 10.0f;
	const float angle = glm::length(rotation) * static_cast<float>(App::getDeltaTime());
	if (angle > 1e-6f) {
		_camera.rotate(glm::angleAxis(angle, glm::normalize(rotation)));
	}
}

void Player::updateOrbitCamera() {
	auto& settings = App::settings.get(settingNames::categories::camera);
	auto& keys = App::settings.get(settingNames::categories::keyBindings)
		.getSubCategory(settingNames::categories::camera);
	const auto isDown = [&keys](const char* name) {
		return ImGui::IsKeyDown(keys.get<ImGuiKey>(name));
	};
	const double zoomSpeed = settings.get<double>(settingNames::camera::zoomSpeed);
	_orbitRadius *= std::exp(-InputEventHandler::mouseScrollWheel * zoomSpeed);
	const double minimumDistance = settings.get<double>(settingNames::camera::minimumOrbitDistance);
	const double maximumDistance = settings.get<double>(settingNames::camera::maximumOrbitDistance);
	_orbitRadius = glm::clamp(_orbitRadius, std::min(minimumDistance, maximumDistance), std::max(minimumDistance, maximumDistance));

	const double sensitivity = settings.get<double>(settingNames::camera::mouseSensitivity);
	const double keyboardSpeed = settings.get<double>(settingNames::camera::keyboardRotationSpeed);
	const double yawInput = static_cast<double>(isDown(settingNames::cameraKeys::rotateRight)) -
		static_cast<double>(isDown(settingNames::cameraKeys::rotateLeft));
	const double pitchInput = static_cast<double>(isDown(settingNames::cameraKeys::rotateUp)) -
		static_cast<double>(isDown(settingNames::cameraKeys::rotateDown));
	_orbitYaw += yawInput * sensitivity * keyboardSpeed;
	_orbitPitch += pitchInput * sensitivity * keyboardSpeed;
	_orbitYaw += InputEventHandler::mouseDelta.x * sensitivity;
	_orbitPitch -= InputEventHandler::mouseDelta.y * sensitivity;
	constexpr double pitchLimit = glm::radians(89.0);
	_orbitPitch = glm::clamp(_orbitPitch, -pitchLimit, pitchLimit);

	glm::vec3 position;
	position.x = static_cast<float>(_orbitRadius * std::cos(_orbitPitch) * std::sin(_orbitYaw));
	position.y = static_cast<float>(_orbitRadius * std::sin(_orbitPitch));
	position.z = static_cast<float>(_orbitRadius * std::cos(_orbitPitch) * std::cos(_orbitYaw));
	const glm::mat4 view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::quat orientation = glm::normalize(glm::quat_cast(glm::mat3(glm::inverse(view))));
	_camera.setTransform(position, orientation);
}
