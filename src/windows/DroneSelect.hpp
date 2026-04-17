#pragma once

#include "../gui/window.hpp"

#include <filesystem>

class DroneSelectWindow
	: public gui::ImGuiWindow {
public:
	DroneSelectWindow() noexcept;

	void render() override;
private:

	void renderFolder(const std::filesystem::path& droneFolder);

	// make this a vector of folders
	std::filesystem::path folder;
};