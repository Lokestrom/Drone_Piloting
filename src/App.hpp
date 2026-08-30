#pragma once

#include <renderer/VulkanApp.hpp>
#include <renderer/Renderer.hpp>
#include <renderer/Camera.hpp>
#include <renderer/Runtime.hpp>
#include "gui/guiApp.hpp"
#include "Drone.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "structures/asyncWorker.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class App {
public:
	static void startup();
	static void shutdown();
	
	static void run();
	static inline int width, height;

	[[nodiscard]]
	static double getDeltaTime() { return dt; };
	[[nodiscard]]
	static GLFWwindow* getGLFWwindow() { return window; };

	[[nodiscard]]
	static renderer::Camera& getCamera() noexcept { return getCurrentPlayer().getCamera(); }
	[[nodiscard]]
	static std::optional<Drone>& getDrone() noexcept { return getCurrentPlayer().getDrone(); }
	[[nodiscard]]
	static renderer::UniformBufferObject getUBO() noexcept {
		renderer::UniformBufferObject ubo = getCurrentPlayer().getUBO();
		ubo.lightSource = map
			? glm::vec4(map->getLightSourcePos(), 1.0f)
			: glm::vec4(0.0f);
		return ubo;
	}

	[[nodiscard]]
	static bool hasActiveWorker() noexcept { return AsyncWorker::hasWork(); }
	static void installMap(
		Map&& replacement,
		std::filesystem::path loadedPath) noexcept;
	[[nodiscard]]
	static const std::filesystem::path& getLoadedMapPath() noexcept { return loadedMapPath; }

	// Should create a player container
	[[nodiscard]]
	static Player& getCurrentPlayer() noexcept {
		assert(hasPlayers() && "Can't get current player if there is no players");
		return *players[currentPlayer];
	}
	[[nodiscard]] 
	static const std::vector<std::unique_ptr<Player>>& getPlayers() noexcept { return players; }
	static void swapToPlayer(const std::string& name) noexcept;
	static void addPlayer(const std::string& name);
	static void removePlayer(const std::string& name) noexcept;
	[[nodiscard]]
	static bool hasPlayers() noexcept { return !players.empty(); };
	[[nodiscard]]
	static bool hasPlayer(const std::string& name) noexcept;

	struct RenderVector {
		glm::vec3 position;
		glm::vec3 direction;
	};
	struct RenderPoint {
		glm::vec3 position;
	};

	static inline settings::Settings settings;
	static inline std::vector<RenderVector> renderVectors;
	static inline std::vector<RenderPoint> renderPoints;

private:
	static inline void loop();
	static void render();
	//should delegate to something else like the inputhandler
	static void updateMouseInput();
	static void startInitialAsyncLoad(
		std::filesystem::path dronePath,
		std::filesystem::path mapPath);
	static void createSettings();
	static void initializeImGuiVulkanBackend();

private:
	static inline vk::raii::DescriptorPool imguiDescriptorPool = nullptr;
	static inline GLFWwindow* window;
	static inline std::vector<std::unique_ptr<Player>> players;
	static inline size_t currentPlayer = 0;
	static inline std::optional<Map> map;
	static inline std::filesystem::path loadedMapPath;
	static inline double dt;
};
