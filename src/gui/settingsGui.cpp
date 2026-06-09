#include "settingsGui.hpp"

#include "../../external/ImGui/imgui.h"
#include <iostream>

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

void color(const std::string& name, glm::vec3& color) {
	ImGui::ColorEdit3(name.c_str(), &color.x, ImGuiColorEditFlags_NoInputs);
}

void keyBindButton(const std::string& name, ImGuiKey& key) {
	static ImGuiKey* waitingForKey = nullptr;
	bool isWaiting = waitingForKey == &key;

	ImGui::TextUnformatted(name.c_str());
	ImGui::SameLine();

	std::string buttonText = ImGui::GetKeyName(key);
	if (isWaiting) {
		buttonText = "Press any key...";
	}

	ImVec2 buttonSize = ImGui::CalcTextSize(buttonText.c_str());
	buttonSize.x += ImGui::GetStyle().FramePadding.x * 10.0f;

	float availWidth = ImGui::GetContentRegionAvail().x;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - buttonSize.x);

	if (ImGui::Button(buttonText.c_str())) {
		if (isWaiting) {
			waitingForKey = nullptr;
			return;
		}
		waitingForKey = &key;
	}

	if (isWaiting) {
		for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
			if (ImGui::IsKeyReleased(static_cast<ImGuiKey>(i))) {
				key = static_cast<ImGuiKey>(i);
				waitingForKey = nullptr;
				break;
			}
		}
	}
}
}