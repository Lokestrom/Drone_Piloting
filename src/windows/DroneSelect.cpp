#include "DroneSelect.hpp"

#include "../App.hpp"
#include "../structures/fileExplorer.hpp"

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
			App::swapDrone(newFolder);
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
}

void DroneSelectWindow::renderFolder(const std::filesystem::path& droneFolder) {
	std::ifstream file(droneFolder / "config.json");
	assert(file && "Cant open config, callers responsibility to check");

	Json jsonData = Json::parse(file, nullptr, true, true);
	if (ImGui::CollapsingHeader(jsonData["name"].get<std::string>().c_str())) {
		ImGui::Text("%s", jsonData["description"].get<std::string>().c_str());
		if (ImGui::Button(("Select##" + jsonData["name"].get<std::string>()).c_str())) {
			App::swapDrone(droneFolder);
		}
	}
}
