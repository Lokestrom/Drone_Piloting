#include "guiApp.hpp"

#include <ranges>
#include <algorithm>
#include <assert.h>
#include <cmath>

#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_glfw.h>
#include "toolBar.hpp"
#include "../App.hpp"

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


	if (enabled && !::App::hasActiveWorker()) {
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

	if (AsyncWorker::hasWork()) [[unlikely]]
		renderAsyncWorkPopup();

	ImGui::Render();
}

void App::renderAsyncWorkPopup() {
	constexpr const char* popup = "Processing...";

	ImGui::OpenPopup(popup);

	ImGui::SetNextWindowSize(ImVec2(500.0f, 0.0f), ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal(
			popup, nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
		const AsyncWorker::Status status = AsyncWorker::status();
		if (status.exception) {
			if (renderAsyncWorkErrorGui(status)) {
				AsyncWorker::dismissError();
				ImGui::CloseCurrentPopup();
			}
		}
		else {
			renderAsyncWorkGui(status);
		}
		ImGui::EndPopup();
	}
}

void App::renderAsyncWorkGui(const AsyncWorker::Status& status) {
	const float progress =
		static_cast<float>(std::fmod(status.elapsedSeconds * 0.55, 1.0));
	ImGui::Text("Processing %s...", status.description.c_str());
	ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
	ImGui::Spacing();
	AsyncWorker::renderGuiSection(status);
	ImGui::Separator();
	ImGui::Text("Elapsed: %.1f seconds", status.elapsedSeconds);
	ImGui::Spacing();
	ImGui::TextDisabled(
		"Rendering and physics are paused while background work is running.");
}

bool App::renderAsyncWorkErrorGui(const AsyncWorker::Status& status) {
	ImGui::Text("Failed to process %s.", status.description.c_str());
	const std::string error = AsyncWorker::exceptionMessage(status.exception);
	if (!error.empty())
		ImGui::TextWrapped("Reason: %s", error.c_str());
	ImGui::TextUnformatted("Check the console for additional details.");
	ImGui::Spacing();
	AsyncWorker::renderErrorGuiSection(status);
	ImGui::Separator();
	if (ImGui::Button("Open console")) {
		openWindow("Console");
		return true;
	}
	ImGui::SameLine();
	return ImGui::Button("OK");
}

void App::shutdown() {
	windows.clear();
}

void App::render() {
	renderer::App::beginPresentationPass();
	ImGui_ImplVulkan_RenderDrawData(
		ImGui::GetDrawData(),
		static_cast<VkCommandBuffer>(renderer::App::currentCommandBuffer()));
	renderer::App::endPresentationPass();
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
