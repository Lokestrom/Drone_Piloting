#pragma once

#include "../gui/window.hpp"

#include <filesystem>

class MapSelectWindow
	: public gui::ImGuiWindow {
public:
	MapSelectWindow() noexcept;

private:
	void _render() override;

	void renderFolder(const std::filesystem::path& mapFolder);
	void selectFolder(const std::filesystem::path& mapFolder);

	// make this a vector of folders
	std::filesystem::path folder;
};
