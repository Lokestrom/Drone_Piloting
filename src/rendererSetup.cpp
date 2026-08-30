#include "rendererSetup.hpp"

#include "App.hpp"
#include "SettingNames.hpp"
#include "console.hpp"
#include "gui/settingsGui.hpp"
#include <renderer/gameObject.hpp>
#include <renderer/Runtime.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

void createRenderingSettings() {
	auto& cameraSettings = App::settings.newCategory(settingNames::categories::camera);
	cameraSettings.emplace<settings::ValueWithRange<double>>(settingNames::camera::fieldOfView, 70.0,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 30.0, 120.0,
		"Vertical field of view in degrees.");
	cameraSettings.emplace<settings::ValueWithRange<double>>(settingNames::camera::moveSpeed, 20.0,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 0.1, 200.0,
		"Free-camera movement speed in world units per second.");
	cameraSettings.emplace<settings::ValueWithRange<double>>(settingNames::camera::keyboardRotationSpeed, 1.0,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 0.1, 10.0,
		"Multiplier applied to camera rotation from keyboard input.");
	cameraSettings.emplace<settings::ValueWithRange<double>>(settingNames::camera::mouseSensitivity, 0.01,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 0.001, 1.0);
	cameraSettings.emplace<settings::ValueWithRange<double>>(settingNames::camera::zoomSpeed, 0.1,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 0.01, 1.0);
	cameraSettings.emplace<settings::ValueWithRange<double>>(settingNames::camera::minimumOrbitDistance, 0.001,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 0.001, 100.0,
		"Closest distance allowed between the orbit camera and its target.");
	cameraSettings.emplace<settings::ValueWithRange<double>>(settingNames::camera::maximumOrbitDistance, 10000.0,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 1.0, 10000.0,
		"Farthest distance allowed between the orbit camera and its target.");

	using KeyValue = settings::Value<ImGuiKey>;
	auto& cameraKeyBinds = App::settings.get(settingNames::categories::keyBindings)
							   .addSubCategory(settingNames::categories::camera);
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::moveForward, ImGuiKey_W, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::moveBackwards, ImGuiKey_S, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::moveLeft, ImGuiKey_A, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::moveRight, ImGuiKey_D, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::moveUp, ImGuiKey_Space, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::moveDown, ImGuiKey_LeftShift, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::rotateLeft, ImGuiKey_LeftArrow, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::rotateRight, ImGuiKey_RightArrow, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::rotateUp, ImGuiKey_UpArrow, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::rotateDown, ImGuiKey_DownArrow, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::rollLeft, ImGuiKey_Q, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::rollRight, ImGuiKey_E, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::freeCamera, ImGuiKey_F, KeyValue::setFunctionT(gui::keyBindButton));
	cameraKeyBinds.emplace<KeyValue>(settingNames::cameraKeys::orbitCamera, ImGuiKey_T, KeyValue::setFunctionT(gui::keyBindButton));

	auto& renderingSettings = App::settings.newCategory(settingNames::categories::rendering);
	renderingSettings.emplace<settings::Value<glm::vec3>>(settingNames::rendering::backgroundColor,
		glm::vec3{ 0.2f }, settings::Value<glm::vec3>::setFunctionT(gui::color));
	renderingSettings.emplace<settings::ValueWithRange<float>>(settingNames::rendering::dynamicObjectViewDistance, 600.0f,
		settings::ValueWithRange<float>::setFunctionT(gui::slider), 10.0f, 10000.0f,
		"Dynamic objects farther than this distance from the camera are not drawn.");
	renderingSettings.emplace<settings::Value<bool>>(settingNames::rendering::shadowsEnabled,
		true, settings::Value<bool>::setFunctionT(gui::checkbox),
		"Render shadows cast by models.");
	renderingSettings.emplace<settings::ValueWithRange<float>>(settingNames::rendering::shadowDistance, 200.0f,
		settings::ValueWithRange<float>::setFunctionT(gui::slider), 10.0f, 10000.0f,
		"Far distance from the camera covered by the shadow cascades.");
	renderingSettings.emplace<settings::ValueWithRange<float>>(settingNames::rendering::textureFullResolutionDistance, 75.0f,
		settings::ValueWithRange<float>::setFunctionT(gui::slider), 1.0f, 5000.0f,
		"Dynamic object textures inside this distance are streamed at full source resolution.");
	renderingSettings.emplace<settings::ValueWithRange<float>>(settingNames::rendering::textureMediumResolutionDistance, 350.0f,
		settings::ValueWithRange<float>::setFunctionT(gui::slider), 1.0f, 10000.0f,
		"Dynamic object textures inside this distance are streamed up to 1024 pixels on their largest side.");
	renderingSettings.emplace<settings::ValueWithRange<float>>(settingNames::rendering::vectorWidth, 0.3f,
		settings::ValueWithRange<float>::setFunctionT(gui::slider), 0.01f, 2.0f,
		"Visual width of force, thrust, and velocity debug arrows.");
	renderingSettings.emplace<settings::ValueWithRange<float>>(settingNames::rendering::vectorLengthScale, 0.1f,
		settings::ValueWithRange<float>::setFunctionT(gui::slider), 0.001f, 1.0f,
		"Scale applied to the length of force, thrust, and velocity debug arrows.");

	renderer::Configuration configuration;
	configuration.applicationName = "Drone piloting";
	configuration.engineName = "Drone piloting renderer";
	renderer::Runtime::configure(std::move(configuration));
	renderer::Runtime::setLogCallback([](renderer::LogLevel level, std::string_view message) {
		Console::Log::Type type = Console::Log::Type::message;
		if (level == renderer::LogLevel::warning) {
			type = Console::Log::Type::warning;
		}
		else if (level == renderer::LogLevel::error) {
			type = Console::Log::Type::error;
		}
		Console::log(type, std::string(message));
	});
}

void syncRenderingConfig() {
	using namespace settingNames;
	auto& configuration = renderer::Runtime::configuration();
	auto& renderingCategory = App::settings.get(categories::rendering);
	configuration.renderer.backgroundColor = renderingCategory.get<glm::vec3>(rendering::backgroundColor);
	configuration.renderer.dynamicObjectViewDistance = renderingCategory.get<float>(rendering::dynamicObjectViewDistance);
	configuration.renderer.shadowsEnabled = renderingCategory.get<bool>(rendering::shadowsEnabled);
	configuration.renderer.shadowDistance = renderingCategory.get<float>(rendering::shadowDistance);
	configuration.renderer.textureFullResolutionDistance = renderingCategory.get<float>(rendering::textureFullResolutionDistance);
	configuration.renderer.textureMediumResolutionDistance = renderingCategory.get<float>(rendering::textureMediumResolutionDistance);
}
