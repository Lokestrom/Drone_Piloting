#pragma once

#include "../gui/window.hpp"

#include <string>

class PlayerMenuWindow
	: public gui::ImGuiWindow {
public:
	PlayerMenuWindow() noexcept;

private:
	void _render() override;

	std::string _newPlayerName;
};