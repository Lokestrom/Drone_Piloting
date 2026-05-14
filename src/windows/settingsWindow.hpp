#pragma once

#include "../gui/window.hpp"

class SettingsWindow
	: public gui::ImGuiWindow {
public:
	SettingsWindow() noexcept;

	void _render() override;
};