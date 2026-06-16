#pragma once

#include "guiApp.hpp"

namespace gui {

void renderToolBar() {
	auto addFunction = App::inMenu ? App::addToMenu : App::addToOverlay;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Info")) {
				addFunction("Info");
			}
			if (ImGui::MenuItem("Drone select")) {
				addFunction("Drone select");
			}
			if (ImGui::MenuItem("Player menu")) {
				addFunction("Player menu");
			}
			if (ImGui::MenuItem("Map select")) {
				addFunction("Map select");
			}
			if (ImGui::MenuItem("Settings")) {
				addFunction("Settings");
			}
			if (ImGui::MenuItem("Console")) {
				addFunction("Console");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

}