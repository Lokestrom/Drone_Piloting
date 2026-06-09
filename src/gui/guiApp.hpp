#pragma once

#include "window.hpp"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

struct ImGui_ImplVulkanH_Window;
namespace gui {

class App {
public:
	static void startup();
	static void generateWindows();
	static void shutdown();

	static void render(ImGui_ImplVulkanH_Window* wd);

	template<typename T>
		requires std::is_base_of_v<ImGuiWindow, T>
	static void addWindow();

	// forces gui enabled
	static void openWindow(const std::string& name);
	
	static void addToOverlay(const std::string& name);
	static void addToMenu(const std::string& name);

	static inline bool enabled;
	static inline bool inMenu;
private:
	// If we want to allow creating multiple windows 
	// of the same type, we would need to change this shit
	
	static inline std::vector<std::unique_ptr<ImGuiWindow>> windows;
	static inline std::vector<ImGuiWindow*> overlayWindows;
	static inline std::vector<ImGuiWindow*> menuWindows;
};

template <typename T>
	requires std::is_base_of_v<ImGuiWindow, T>
inline void App::addWindow() {
	windows.emplace_back(std::make_unique<T>());
}

}