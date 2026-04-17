#include "DroneSelect.hpp"

#include "../App.hpp"

#include <fstream>
#include <json.hpp>

using Json = nlohmann::json;

DroneSelectWindow::DroneSelectWindow() noexcept 
	: gui::ImGuiWindow("Drone select", true)
	, folder(ASSET_DIR "Drones/") {
}

void DroneSelectWindow::render() {
	begin();

	for (const auto& entry : std::filesystem::directory_iterator(folder)) {
		if (entry.is_directory()) {
			renderFolder(entry.path());
		}
	}

	end();
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
