#include "settingsGui.hpp"

#include "../../external/ImGui/imgui.h"

#include <algorithm>

namespace {

constexpr float inputWidth = 200.0f;

void renderSettingName(const std::string& name) {
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(name.c_str());
	ImGui::SameLine();
}

float alignInputToRight(float desiredWidth) {
	const float availableWidth = std::max(ImGui::GetContentRegionAvail().x, 1.0f);
	const float width = std::min(desiredWidth, availableWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth - width);
	return width;
}

std::string inputID(const std::string& name) {
	return "##" + name;
}

}

namespace gui {

void checkbox(const std::string& name, bool& value) {
	renderSettingName(name);
	alignInputToRight(ImGui::GetFrameHeight());
	ImGui::Checkbox(inputID(name).c_str(), &value);
}

void input(const std::string& name, double& value) {
	renderSettingName(name);
	ImGui::SetNextItemWidth(alignInputToRight(inputWidth));
	ImGui::InputDouble(inputID(name).c_str(), &value);
}

void slider(const std::string& name, int& value, const int& min, const int& max) {
	renderSettingName(name);
	ImGui::SetNextItemWidth(alignInputToRight(inputWidth));
	ImGui::SliderInt(inputID(name).c_str(), &value, min, max);
}

void slider(const std::string& name, float& value, const float& min, const float& max) {
	renderSettingName(name);
	ImGui::SetNextItemWidth(alignInputToRight(inputWidth));
	ImGui::SliderFloat(inputID(name).c_str(), &value, min, max);
}
void slider(const std::string& name, double& value, const double& min, const double& max) {
	renderSettingName(name);
	ImGui::SetNextItemWidth(alignInputToRight(inputWidth));
	ImGui::SliderDouble(inputID(name).c_str(), &value, min, max);
}

void color(const std::string& name, glm::vec3& color) {
	renderSettingName(name);
	ImGui::SetNextItemWidth(alignInputToRight(ImGui::GetFrameHeight()));
	ImGui::ColorEdit3(inputID(name).c_str(), &color.x, ImGuiColorEditFlags_NoInputs);
}

void keyBindButton(const std::string& name, ImGuiKey& key) {
	static ImGuiKey* waitingForKey = nullptr;
	bool isWaiting = waitingForKey == &key;

	renderSettingName(name);

	std::string buttonText = ImGui::GetKeyName(key);
	if (isWaiting) {
		buttonText = "Press any key...";
	}

	ImVec2 buttonSize = ImGui::CalcTextSize(buttonText.c_str());
	buttonSize.x += ImGui::GetStyle().FramePadding.x * 10.0f;

	alignInputToRight(buttonSize.x);

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
