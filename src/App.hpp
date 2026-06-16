#pragma once

#include "rendering/VulkanApp.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Camera.hpp"
#include "gui/guiApp.hpp"
#include "Drone.hpp"
#include "Map.hpp"
#include "Player.hpp"

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
	static vulkan::Camera& getCamera() noexcept { return getCurrentPlayer().getCamera(); }
	[[nodiscard]]
	static std::optional<Drone>& getDrone() noexcept { return getCurrentPlayer().getDrone(); }
	[[nodiscard]]
	static vulkan::UniformBufferObject getUBO() noexcept { 
		vulkan::UniformBufferObject ubo = getCurrentPlayer().getUBO();
		ubo.lightSource = glm::vec4(map.getLightSourcePos(),1.0);
		return ubo;
	}

	static void swapDrone(std::filesystem::path folderPath) { getCurrentPlayer().SwapDrone(folderPath); }
	[[nodiscard]]
	static bool swapMap(std::filesystem::path folderPath) {
		map.unload();
		return map.load(folderPath);
	}

	// Should create a player container
	[[nodiscard]]
	static Player& getCurrentPlayer() noexcept {
		assert(hasPlayers() && "Can't get current player if there is no players");
		return *players[currentPlayer];
	}
	[[nodiscard]] 
	static const std::vector<std::unique_ptr<Player>>& getPlayers() noexcept { return players; }
	static void swapPlayer(const std::string& name) noexcept;
	static void addPlayer(const std::string& name);
	static void removePlayer(const std::string& name) noexcept;
	[[nodiscard]]
	static bool hasPlayers() noexcept { return !players.empty(); };
	[[nodiscard]]
	static bool hasPlayer(const std::string& name) noexcept;

	struct RenderVector {
		glm::vec3 position;
		glm::vec3 dir;
		glm::vec4 color = glm::vec4(1, 0, 0, 1);
	};
	struct RenderPoint {
		glm::vec3 position;
		glm::vec4 color = glm::vec4(1, 0, 0, 1);
	};

	static inline settings::Settings settings;
	static inline std::vector<RenderVector> renderVectors;
	static inline std::vector<RenderPoint> renderPoints;
	static inline glm::vec2 vectorScale;

private:
	static inline void loop();
	static void render();
	//should delegate to something else like the inputhandler
	static void updateMouseInput();
	static void createSettings();

private:
	static inline ImGui_ImplVulkanH_Window* wd;
	static inline GLFWwindow* window;
	static inline std::vector<std::unique_ptr<Player>> players;
	static inline size_t currentPlayer = 0;
	static inline Map map;
	static inline double dt;
};