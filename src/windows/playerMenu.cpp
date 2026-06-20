#include "playerMenu.hpp"

#include <ImGui/imgui.h>
#include "../App.hpp"

PlayerMenuWindow::PlayerMenuWindow() noexcept 
	: gui::ImGuiWindow("Player menu", true) {

}

void PlayerMenuWindow::_render() {
	if (ImGui::Button("New player")) {
		ImGui::OpenPopup("Create new player");
	}

	if (ImGui::BeginPopupModal("Create new player")) {
		bool nameExists = App::hasPlayer(_newPlayerName);
		bool nameEmpty = _newPlayerName.empty();

		if (nameExists) {
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
		}
		ImGui::InputText("Player name: ", &_newPlayerName);
		if (nameExists) {
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}

		ImGui::BeginDisabled(nameExists || nameEmpty);
		if (ImGui::Button("Create")) {
			App::addPlayer(_newPlayerName);
			_newPlayerName = {};
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::EndPopup();
	}

	std::string toDelete;
	
	for (auto& player : App::getPlayers()) {
		if (!ImGui::CollapsingHeader(player->name().c_str())) {
			continue;
		}

		// TODO: display loaded drone
		// TODO: implement some setting tab to change pos and other things
		
		if (player->getDrone().has_value())
			ImGui::TextUnformatted("Type: Drone");
		else
			ImGui::TextUnformatted("Type: Camera");

		if (player.get() == &App::getCurrentPlayer()) {
			ImGui::TextUnformatted("Selected");
			continue;
		}
		if (ImGui::Button(("Select##" + player->name()).c_str())) {
			App::swapToPlayer(player->name());
		}
		ImGui::SameLine();
		// TODO: make indexed or use ptr, avoids heap allocation
		if (ImGui::Button(("Delete##" + player->name()).c_str())) {
			toDelete = player->name();
		}
	}

	if (!toDelete.empty()) {
		App::removePlayer(toDelete);
	}


}