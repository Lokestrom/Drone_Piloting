#include "settingsWindow.hpp"

#include "../Settings.hpp"

SettingsWindow::SettingsWindow() noexcept
	: gui::ImGuiWindow("Settings", true) {
}

void SettingsWindow::_render() {
	for (auto& category : settings::Settings()) {
		if(ImGui::CollapsingHeader(category.first.c_str()))
			for (auto& value : category.second) {
				value.second->set();
			}
	}
}