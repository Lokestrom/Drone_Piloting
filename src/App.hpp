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

void createSettings();

class App {
public:
	static void startup();
	static void shutdown();
	
	static void run();
	static inline int width, height;

	static double getDeltaTime() { return dt; };
	static GLFWwindow* getGLFWwindow() { return window; };

	static vulkan::Camera& getCamera() noexcept { return player->getCamera(); }
	static std::optional<Drone>& getDrone() noexcept { return player->getDrone(); }
	static vulkan::UniformBufferObject getUBO() noexcept { 
		vulkan::UniformBufferObject ubo = player->getUBO();
		ubo.lightSource = glm::vec4(map.getLightSourcePos(),1.0);
		return ubo;
	}

	static void swapDrone(std::filesystem::path folderPath) { player->SwapDrone(folderPath); }
	static bool swapMap(std::filesystem::path folderPath) {
		map.unload();
		return map.load(folderPath);
	}

	struct RenderVector {
		glm::vec3 position;
		glm::vec3 dir;
		glm::vec4 color = glm::vec4(1, 0, 0, 1);
	};
	struct RenderPoint {
		glm::vec3 position;
		glm::vec4 color = glm::vec4(1, 0, 0, 1);
	};

	static inline std::vector<RenderVector> renderVectors;
	static inline std::vector<RenderPoint> renderPoints;
	static inline glm::vec2 vectorScale;

private:
	static inline void loop();
	static void render();
	//should delegate to something else like the inputhandler
	static void updateMouseInput();

private:
	static inline ImGui_ImplVulkanH_Window* wd;
	static inline GLFWwindow* window;
	static inline std::unique_ptr<Player> player;
	static inline Map map;
	static inline double dt;
};