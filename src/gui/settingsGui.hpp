#include "../../external/ImGui/imgui.h"

#include <string>

namespace gui {

void checkbox(const std::string& name, bool& value) {
	ImGui::Checkbox(name.c_str(), &value);
}

void input(const std::string& name, double& value) {
	ImGui::InputDouble(name.c_str(), &value);
}

void slider(const std::string& name, float& value, const float& min, const float& max) {
	ImGui::SliderFloat(name.c_str(), &value, min, max);
}
void slider(const std::string& name, double& value, const double& min, const double& max) {
	ImGui::SliderDouble(name.c_str(), &value, min, max);
}

}