#include "settingsWindow.hpp"

#include "../App.hpp"
#include "../Input/InputEventHandler.hpp"

SettingsWindow::SettingsWindow() noexcept
	: gui::ImGuiWindow("Settings", true) {
}

void SettingsWindow::render() {
	begin();

	ImGui::InputFloat("Vector radius scale: ", &App::vectorScale.x);
	ImGui::InputFloat("Vector length scale: ", &App::vectorScale.y);

	end();
}