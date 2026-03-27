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

	static double getDeltaTime() { return dt; };
	static GLFWwindow* getGLFWwindow() { return window; };

	static Camera& getCamera() noexcept { return player.getCamera(); }
	static std::optional<Drone>& getDrone() noexcept { return player.getDrone(); }
	static vulkan::UniformBufferObject getUBO() noexcept { 
		vulkan::UniformBufferObject ubo = player.getUBO(); 
		ubo.lightSource = glm::vec4(map.getLightSourcePos(),1.0);
		return ubo;
	}

private:
	static void render();

private:
	static inline ImGui_ImplVulkanH_Window* wd;
	static inline GLFWwindow* window;
	static inline Player player;
	static inline Map map;
	static inline double dt;
};