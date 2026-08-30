#include "debugObjects.hpp"

#include "App.hpp"
#include "SettingNames.hpp"
#include "console.hpp"
#include "gui/settingsGui.hpp"
#include "rendering/gameObject.hpp"
#include "rendering/Runtime.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace {
std::vector<renderer::ID> vectorObjects;
std::vector<renderer::ID> pointObjects;

[[nodiscard]] renderer::ID createDebugObject(const std::filesystem::path& modelPath) {
	assert(std::filesystem::exists(modelPath) && "Model file does not exist");
	return renderer::GameObjectContainer::Add(renderer::GameObject{
		renderer::ModelCache::loadModel(modelPath),
		glm::vec3(0.0f),
		glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f)
	});
}

void ensureDebugObjectCount(
	std::vector<renderer::ID>& objectContainer,
	size_t requiredCount,
	const std::filesystem::path& modelPath) {
	objectContainer.reserve(requiredCount);
	while (objectContainer.size() < requiredCount) {
		objectContainer.push_back(createDebugObject(modelPath));
	}
}
}

void syncDebugObjects() {
	ensureDebugObjectCount(
		vectorObjects,
		App::renderVectors.size(),
		std::filesystem::path(ASSET_DIR) / "other/vectorArrow.obj");
	ensureDebugObjectCount(
		pointObjects,
		App::renderPoints.size(),
		std::filesystem::path(ASSET_DIR) / "other/point.obj");

	assert(vectorObjects.size() >= App::renderVectors.size());
	assert(pointObjects.size() >= App::renderPoints.size());

	auto& renderingSettings = App::settings.get(settingNames::categories::rendering);
	const float vectorWidth = renderingSettings.get<float>(settingNames::rendering::vectorWidth);
	const float vectorLengthScale = renderingSettings.get<float>(settingNames::rendering::vectorLengthScale);
	for (size_t index = 0; index < vectorObjects.size(); ++index) {
		renderer::GameObject& object = renderer::GameObjectContainer::get(vectorObjects[index]);
		if (index >= App::renderVectors.size() || glm::length2(App::renderVectors[index].direction) == 0.0f) {
			object.scale = glm::vec3(0.0f);
			continue;
		}
		const App::RenderVector& vector = App::renderVectors[index];
		object.position = vector.position;
		object.orientation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(vector.direction));
		object.scale = glm::vec3(
			vectorWidth,
			glm::length(vector.direction) * vectorLengthScale,
			vectorWidth);
	}

	for (size_t index = 0; index < pointObjects.size(); ++index) {
		renderer::GameObject& object = renderer::GameObjectContainer::get(pointObjects[index]);
		if (index >= App::renderPoints.size()) {
			object.scale = glm::vec3(0.0f);
			continue;
		}
		object.position = App::renderPoints[index].position;
		object.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		object.scale = glm::vec3(1.0f);
	}
}

void destroyDebugObjects() noexcept {
	renderer::GameObjectContainer::remove(vectorObjects);
	renderer::GameObjectContainer::remove(pointObjects);
	vectorObjects.clear();
	pointObjects.clear();
}
