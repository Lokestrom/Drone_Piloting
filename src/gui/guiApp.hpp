#pragma once

#include "window.hpp"

#include <vector>
#include <memory>
#include <string>

struct ImGui_ImplVulkanH_Window;
namespace gui {

class App {
public:
	static void startup();
	static void generateWindows();

	static void render(ImGui_ImplVulkanH_Window* wd);

	static void openWindow(std::string name);
	template<typename T>
		requires std::is_base_of_v<ImGuiWindow, T>
	static void addWindow();
	
	static inline bool active;
private:
	static inline std::vector<std::unique_ptr<ImGuiWindow>> windows;
};

template <typename T>
	requires std::is_base_of_v<ImGuiWindow, T>
inline void App::addWindow() {
	windows.emplace_back(std::make_unique<T>());
}

}