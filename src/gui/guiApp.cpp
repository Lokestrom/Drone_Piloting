#include "guiApp.hpp"

#include <ranges>
#include <algorithm>
#include <assert.h>

#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_glfw.h>
#include "toolBar.hpp"

namespace gui {
void App::startup() {
	enabled = false;
	inMenu = false;
}
void App::generateWindows() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);


	if (enabled) {
		renderToolBar();
		if (inMenu) {
			for (auto i = 0uz; i < menuWindows.size(); ++i) {
				auto window = menuWindows[i];
				if (!window->isOpen()) {
					menuWindows.erase(menuWindows.begin() + i--);
					continue;
				}
				window->render();
			}
		}
		else {
			for (auto i = 0uz; i < overlayWindows.size(); ++i) {
				auto window = overlayWindows[i];
				if (!window->isOpen()) {
					overlayWindows.erase(overlayWindows.begin() + i--);
					continue;
				}
				window->render();
			}
		}
	}

	ImGui::Render();
}

void App::shutdown() {
	windows.clear();
}

void App::render(ImGui_ImplVulkanH_Window* wd) {
	
	ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
	{
		vk::RenderPassBeginInfo info;
		info.renderPass = wd->RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = wd->Width;
		info.renderArea.extent.height = wd->Height;
		info.clearValueCount = 0;
		static_cast<vk::CommandBuffer>(fd->CommandBuffer).beginRenderPass(&info, vk::SubpassContents::eInline);
	}

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

	static_cast<vk::CommandBuffer>(fd->CommandBuffer).endRenderPass();
}
void App::openWindow(const std::string& name) {
	enabled = true;
	if (inMenu) {
		addToMenu(name);
	}
	else {
		addToOverlay(name);
	}
}


void App::addToOverlay(const std::string& name) {
	if (std::ranges::find_if(overlayWindows, [&](const ImGuiWindow* window) { return window->getName() == name; }) != overlayWindows.end()) {
		ImGui::SetWindowFocus(name.c_str());
		return;
	}
	auto window = std::ranges::find_if(windows, [&](const std::unique_ptr<ImGuiWindow>& window) { return window->getName() == name; });
	assert(window != windows.end() && "Tried adding a window to the overlay that does not exist");
	window->get()->open();
	overlayWindows.push_back(window->get());
}

void App::addToMenu(const std::string& name) {
	if (std::ranges::find_if(menuWindows, [&](const ImGuiWindow* window) { return window->getName() == name; }) != menuWindows.end()) {
		ImGui::SetWindowFocus(name.c_str());
		return;
	}
	auto window = std::ranges::find_if(windows, [&](const std::unique_ptr<ImGuiWindow>& window) { return window->getName() == name; });
	assert(window != windows.end() && "Tried adding a window to the menu that does not exist");
	window->get()->open();
	menuWindows.push_back(window->get());
}

}