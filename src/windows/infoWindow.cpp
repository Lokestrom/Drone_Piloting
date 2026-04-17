#include "infoWindow.hpp"

#include "../App.hpp"
#include "../Input/InputEventHandler.hpp"

InfoWindow::InfoWindow() noexcept
	: gui::ImGuiWindow("Info", true) {
}

void InfoWindow::render() {
	begin();

	ImGui::InputFloat3("Camera position", &App::getCamera().getPositionRef().x);
	if (App::getDrone().has_value()) {
		ImGui::InputFloat3("Drone position", &App::getDrone()->getPosition().x);
		ImGui::InputFloat3("Drone velocity", &App::getDrone()->getVelocity().x);
		ImGui::InputFloat4("Drone orientation", &App::getDrone()->getOrientation().x);
		ImGui::InputFloat3("Drone rotational velocity", &App::getDrone()->getRotationalVelocity().x);
	}

	end();
}