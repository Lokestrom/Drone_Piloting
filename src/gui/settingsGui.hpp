#pragma once

#include <string>
#include <glm/glm/vec3.hpp>
#include <ImGui/imgui.h>

namespace gui {

void checkbox(const std::string& name, bool& value);

void input(const std::string& name, double& value);

void slider(const std::string& name, int& value, const int& min, const int& max);

void slider(const std::string& name, float& value, const float& min, const float& max);

void slider(const std::string& name, double& value, const double& min, const double& max);

void color(const std::string& name, glm::vec3& color);

void keyBindButton(const std::string& name, ImGuiKey& key);

}
