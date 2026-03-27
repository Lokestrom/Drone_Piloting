#pragma once

#include "../gui/window.hpp"

class InfoWindow 
	: public gui::ImGuiWindow 
{
public:
	InfoWindow() noexcept;

	void render() override;
};