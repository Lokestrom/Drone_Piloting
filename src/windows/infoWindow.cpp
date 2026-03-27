#include "infoWindow.hpp"
#include "infoWindow.hpp"

#include "../App.hpp"

InfoWindow::InfoWindow() noexcept 
	: gui::ImGuiWindow("Info", true) {
}

void InfoWindow::render() {
	begin();

	ImGui::InputFloat3("Camera position", &App::getCamera().getPositionRef().x);
	if (App::getDrone().has_value()) {
		ImGui::InputFloat3("Drone position", &App::getDrone()->getPosition().x);
		ImGui::InputFloat3("Drone velocity", &App::getDrone()->getVelocity().x);
	}

	end();
}