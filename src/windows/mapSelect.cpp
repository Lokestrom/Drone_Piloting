#include "mapSelect.hpp"

#include "../App.hpp"
#include "../structures/fileExplorer.hpp"

#include <fstream>
#include <json.hpp>

using Json = nlohmann::json;

MapSelectWindow::MapSelectWindow() noexcept
	: gui::ImGuiWindow("Map select", true)
	, folder(ASSET_DIR "Maps/") {
}

void MapSelectWindow::_render() {
#ifdef _WIN32
	if (ImGui::Button("Find map")) {
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

	if (ImGui::BeginPopupModal("Map failed to load")) {
		std::string message =
			"The map:\n\"" + loadedMap.string() +
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

void MapSelectWindow::renderFolder(const std::filesystem::path& mapFolder) {
	std::ifstream file(mapFolder / "config.json");
	assert(file && "Cant open config, callers responsibility to check");

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

	if (ImGui::CollapsingHeader((std::string(jsonData["name"]) + "##" + mapFolder.string()).c_str())) {
		if (jsonData.contains("description") &&
			jsonData["description"].is_string()) {
			ImGui::TextUnformatted(
				jsonData["description"].get_ref<const std::string&>().c_str());
		}

		if (!failed && loadedMap == mapFolder) {
			ImGui::TextUnformatted("Currently selected");
			return;
		}

		if (ImGui::Button(("Select##" + mapFolder.string()).c_str())) {
			selectFolder(mapFolder);
		}
	}
}

void MapSelectWindow::selectFolder(const std::filesystem::path& mapFolder) {
	loadedMap = mapFolder;
	failed = false;
	if (!App::swapMap(mapFolder)) {
		failed = true;
		ImGui::OpenPopup("Map failed to load");
	}
}
