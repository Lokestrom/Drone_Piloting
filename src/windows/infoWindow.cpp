#include "infoWindow.hpp"

#include "../App.hpp"
#include "../Input/InputEventHandler.hpp"

InfoWindow::InfoWindow() noexcept
	: gui::ImGuiWindow("Info", true) {
}

void InfoWindow::_render() {
	ImGui::InputFloat3("Camera position", &App::getCamera().getPositionRef().x);
	if (App::getDrone().has_value()) {
		ImGui::InputFloat3("Drone position", &App::getDrone()->getPosition().x);
		ImGui::InputFloat3("Drone velocity", &App::getDrone()->getVelocity().x);
		ImGui::InputFloat4("Drone orientation", &App::getDrone()->getOrientation().x);
		ImGui::InputFloat3("Drone rotational velocity", &App::getDrone()->getRotationalVelocity().x);
	}

	if (App::getDrone()->hasSettings()) {
		if (ImGui::CollapsingHeader("Settings")) {
			for (size_t i = 0; i < App::getDrone()->getSettings()->count; ++i) {
				ImGui::SliderFloat(App::getDrone()->getSettings()->names[i], App::getDrone()->getSettings()->values[i], 0.0f, 5.0f, "%.2f");
			}
		}
	}
}