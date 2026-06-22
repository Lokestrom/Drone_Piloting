#pragma once

#include "../gui/window.hpp"
#include "../Settings.hpp"

#include <filesystem>

class DroneSelectWindow
	: public gui::ImGuiWindow {
public:
	DroneSelectWindow() noexcept;

	static void createSettings();

private:
	void _render() override;

	void renderFolder(const std::filesystem::path& droneFolder);

	void selectFolder(const std::filesystem::path& droneFolder);

	// make this a vector of folders
	std::filesystem::path folder;
	std::filesystem::path lastFailedDrone;
	settings::ValueHandle<bool> hideDroneSafetyWarning;
};