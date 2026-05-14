#include "window.hpp"

#include <ImGui/imgui.h>

gui::ImGuiWindow::ImGuiWindow(std::string_view name, bool open) noexcept 
	: _name(name), _open(open)
{}

void gui::ImGuiWindow::render() {
	ImGui::Begin(_name.data(), &_open);

	_render();

	ImGui::End();
}