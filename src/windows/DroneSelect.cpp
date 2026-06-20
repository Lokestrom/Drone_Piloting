#include "DroneSelect.hpp"

#include "../App.hpp"
#include "../structures/fileExplorer.hpp"
#include "../console.hpp"

#include <fstream>
#include <json.hpp>

using Json = nlohmann::json;

DroneSelectWindow::DroneSelectWindow() noexcept 
	: gui::ImGuiWindow("Drone select", true)
	, folder(ASSET_DIR "Drones/") {
}

void DroneSelectWindow::_render() {
#ifdef _WIN32
	if (ImGui::Button("Find drone")) {
		std::filesystem::path newFolder = OpenFileExplorer();
		if (!newFolder.empty()) {
			selectFolder(newFolder);
		}
	}
#endif
	for (const auto& entry : std::filesystem::directory_iterator(folder)) {
		if (!entry.is_directory()) {
			continue;
		}

		const auto configPath = entry.path() / "config.json";
		if (!std::filesystem::exists(configPath)) {
			continue;
		}

		renderFolder(entry.path());
	}

	if (ImGui::BeginPopupModal("Drone failed to load")) {
		std::string message =
			"The drone:\n\"" + lastFailedDrone.string() +
			"\"\nfailed to load.\n\nCheck the console for errors.";

		ImGui::TextUnformatted(message.c_str());

		ImGui::Separator();

		if (ImGui::Button("Open console")) {
			gui::App::openWindow("Console");
		}
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void DroneSelectWindow::renderFolder(const std::filesystem::path& droneFolder) {
	// TODO: create user feed back for failed file open and json
	std::ifstream file(droneFolder / "config.json");
	if (!file) {
		return;
	}

	Json jsonData;
	try {
		jsonData = Json::parse(file, nullptr, true, true);
	}
	catch (const Json::exception&) {
		return;
	}

	if (!jsonData.contains("name") || !jsonData["name"].is_string()) {
		return;
	}

	if (ImGui::CollapsingHeader(jsonData["name"].get<std::string>().c_str())) {
		if (jsonData.contains("description") &&
			jsonData["description"].is_string()) {
			ImGui::TextUnformatted(
				jsonData["description"].get_ref<const std::string&>().c_str());
		}

		if (ImGui::Button(("Select##" + jsonData["name"].get<std::string>()).c_str())) {
			selectFolder(droneFolder);
		}
	}
}

void DroneSelectWindow::selectFolder(const std::filesystem::path& droneFolder) {
	lastFailedDrone = droneFolder;
	if (!App::swapDrone(droneFolder)) {
		ImGui::OpenPopup("Drone failed to load");
	}
}
