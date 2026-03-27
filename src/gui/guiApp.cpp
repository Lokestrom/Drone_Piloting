#include "guiApp.hpp"

#include <ranges>
#include <algorithm>
#include <assert.h>

#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_glfw.h>

namespace gui {
void App::startup() {
}
void App::generateWindows() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	for (auto& window : windows) {
		if (window->isOpen())
			window->render();
	}

	ImGui::Render();
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

void App::openWindow(std::string name) {
	auto window = std::ranges::find_if(windows, [&](std::unique_ptr<ImGuiWindow>& window) { return window->getName() == name; });
	assert(window != windows.end() && "Tried opening a window that does not exist");
	window->get()->open();
}

}