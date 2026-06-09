#include "settingsWindow.hpp"

#include "../Settings.hpp"
#include "../App.hpp"

SettingsWindow::SettingsWindow() noexcept
	: gui::ImGuiWindow("Settings", true) {
}


void renderCategory(settings::SettingsCategory& category) {
	if (ImGui::TreeNodeEx((void*)&category, {}, "%s", category.name.c_str())) {
		for (auto& value : category.getValues()) {
			ImGui::Separator();
			value->set();
		}

		for (auto& subCategory : category.getSubCategories()) {
			ImGui::Separator();
			renderCategory(*subCategory);
		}

		ImGui::TreePop();
	}
}

void SettingsWindow::_render() {
	for (auto& category : App::settings) {
		renderCategory(*category);
		ImGui::Separator();
	}
}