#include "window.hpp"

#include <ImGui/imgui.h>

gui::ImGuiWindow::ImGuiWindow(std::string_view name, bool open) noexcept 
	: _name(name), _open(open)
{}

void gui::ImGuiWindow::render() {
	begin();
	end();
}

void gui::ImGuiWindow::begin() {
	ImGui::Begin(_name.data(), &_open);
}

void gui::ImGuiWindow::end() {
	ImGui::End();
}
