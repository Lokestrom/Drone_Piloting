#include "mapSelect.hpp"

#include "../App.hpp"
#include "../structures/fileExplorer.hpp"
#include "../gui/asyncWorkerGui.hpp"
#include <renderer/VulkanApp.hpp>

#include <fstream>
#include <json.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

using Json = nlohmann::json;

MapSelectWindow::MapSelectWindow() noexcept
	: gui::ImGuiWindow("Map select", true)
	, folder(ASSET_DIR "Maps/") {
}

void MapSelectWindow::_render() {
#ifdef _WIN32
	if (ImGui::Button("Find map")) {
		std::filesystem::path newFolder = OpenFileExplorer();
		if (!newFolder.empty() && std::filesystem::is_directory(newFolder)) {
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

}

void MapSelectWindow::renderFolder(const std::filesystem::path& mapFolder) {
	// TODO: create user feed back for failed file open and json
	std::ifstream file(mapFolder / "config.json");
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

	if (ImGui::CollapsingHeader((std::string(jsonData["name"]) + "##" + mapFolder.string()).c_str())) {
		if (jsonData.contains("description") &&
			jsonData["description"].is_string()) {
			ImGui::TextUnformatted(
				jsonData["description"].get_ref<const std::string&>().c_str());
		}

		if (App::getLoadedMapPath() == mapFolder) {
			ImGui::TextUnformatted("Currently selected");
			return;
		}

		if (ImGui::Button(("Select##" + mapFolder.string()).c_str())) {
			selectFolder(mapFolder);
		}
	}
}

void MapSelectWindow::selectFolder(const std::filesystem::path& mapFolder) {
	auto workerGui = std::make_shared<gui::AsyncWorkerGui>(
		std::vector<gui::AsyncWorkerGui::Path>{ { "Path", mapFolder } });

	AsyncWorker::submit(AsyncWorker::WorkOrder{
		.description = "map",
		.work = [
			mapFolder,
			workerGui]() mutable -> AsyncWorker::CompletionFn {
			renderer::App::waitIdle();
			workerGui->setPhase("Loading models and textures");

			Map loadedMap;
			if (!loadedMap.load(mapFolder))
				throw std::runtime_error("The selected map folder could not be loaded.");

			workerGui->setPhase("Installing loaded map");
			return AsyncWorker::CompletionFn{
				[mapFolder, loadedMap = std::move(loadedMap)]() mutable {
					App::installMap(std::move(loadedMap), mapFolder);
				}
			};
		},
		.detailsGui = [workerGui](const AsyncWorker::Status& status) {
			workerGui->renderDetails(status);
		},
		.errorDetailsGui = [workerGui](const AsyncWorker::Status& status) {
			workerGui->renderErrorDetails(status);
		}
	});
}
