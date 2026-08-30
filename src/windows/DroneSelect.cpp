#include "DroneSelect.hpp"

#include "../App.hpp"
#include "../structures/fileExplorer.hpp"
#include "../gui/settingsGui.hpp"
#include "../gui/asyncWorkerGui.hpp"
#include <renderer/VulkanApp.hpp>
#include "../SettingNames.hpp"

#include <algorithm>
#include <fstream>
#include <json.hpp>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

using Json = nlohmann::json;

namespace {
constexpr const char* hideDroneSafetyWarningSetting = "Hide drone selection unsafe warning";
constexpr const char* droneSafetyWarningPopup = "Drone selection safety warning";
}

DroneSelectWindow::DroneSelectWindow() noexcept 
	: gui::ImGuiWindow("Drone select", true)
	, folder(ASSET_DIR "Drones/")
	, hideDroneSafetyWarning(
		  App::settings.get(settingNames::categories::safety)
			.get<bool>(hideDroneSafetyWarningSetting)
			.getHandle()) {
}

void DroneSelectWindow::createSettings() {
	auto& safetySettings = App::settings.get(settingNames::categories::safety);
	safetySettings.emplace<settings::Value<bool>>(hideDroneSafetyWarningSetting,
		false, settings::Value<bool>::setFunctionT(gui::checkbox),
		"Suppresses the warning shown before selecting code from a drone folder.");
}

void DroneSelectWindow::_render() {
	if (!hideDroneSafetyWarning.get() && ImGui::IsWindowAppearing()) {
		ImGui::OpenPopup(droneSafetyWarningPopup);
	}
#ifdef _WIN32
	if (ImGui::Button("Find drone")) {
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

	if (ImGui::BeginPopupModal(droneSafetyWarningPopup)) {
		ImGui::TextWrapped(
			"Selecting a drone will load compiled code from the drone folder. "
			"There is no guards and the binary has the same premisions as this app. "
			"Only select drone folders from sources you trust."
			"This is the only warning and when selecting the drone it will just load.");

		ImGui::Separator();

		ImGui::Checkbox("Don't show this warning again", &hideDroneSafetyWarning.get());

		if (ImGui::Button("Understand")) {
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
	const std::string playerName = App::getCurrentPlayer().name();
	std::optional<API::DroneState> previousState;
	if (App::getCurrentPlayer().getDrone())
		previousState = App::getCurrentPlayer().getDrone()->getState();

	auto workerGui = std::make_shared<gui::AsyncWorkerGui>(
		std::vector<gui::AsyncWorkerGui::Path>{ { "Path", droneFolder } });

	AsyncWorker::submit(AsyncWorker::WorkOrder{
		.description = "drone",
		.work = [
			droneFolder,
			playerName,
			previousState,
			workerGui]() mutable -> AsyncWorker::CompletionFn {
			renderer::App::waitIdle();
			workerGui->setPhase("Loading models, configuration, and code");

			Drone loadedDrone;
			const bool loaded = previousState
				? loadedDrone.load(droneFolder, *previousState)
				: loadedDrone.load(droneFolder);
			if (!loaded)
				throw std::runtime_error(
					"The selected drone folder could not be loaded.");

			workerGui->setPhase("Installing loaded drone");
			return AsyncWorker::CompletionFn{
				[
					playerName,
					loadedDrone = std::move(loadedDrone)]() mutable {
					const auto player = std::ranges::find_if(
						App::getPlayers(),
						[&](const std::unique_ptr<Player>& candidate) {
							return candidate->name() == playerName;
						});
					assert(player != App::getPlayers().end() && "Player not found, no player may be destroyed during a async operation");

					player->get()->replaceDrone(std::move(loadedDrone));
				}
			};
		},
		.detailsGui = [workerGui](const AsyncWorker::Status& status) {
			workerGui->renderDetails(status);
		},
		.errorDetailsGui = [workerGui](const AsyncWorker::Status& status) {
			workerGui->renderErrorDetails(status);
		},
	});
}
