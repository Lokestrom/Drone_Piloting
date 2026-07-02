#include "settingsWindow.hpp"

#include "../Settings.hpp"
#include "../App.hpp"

SettingsWindow::SettingsWindow() noexcept
	: gui::ImGuiWindow("Settings", true) {
}

namespace {

void showHintWhenNameIsHovered(const settings::IValue& value, const ImVec2& namePosition) {
	if (value.hint().empty())
		return;

	const float nameWidth = ImGui::CalcTextSize(value.name().c_str()).x;
	const ImVec2 nameEnd{
		namePosition.x + nameWidth,
		namePosition.y + ImGui::GetFrameHeight()
	};
	if (!ImGui::IsMouseHoveringRect(namePosition, nameEnd))
		return;

	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::TextUnformatted(value.hint().c_str());
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

void renderCategory(settings::SettingsCategory& category) {
	if (ImGui::TreeNodeEx((void*)&category, {}, "%s", category.name.c_str())) {
		for (auto& value : category.getValues()) {
			ImGui::Separator();
			const ImVec2 namePosition = ImGui::GetCursorScreenPos();
			value->set();
			showHintWhenNameIsHovered(*value, namePosition);
		}

		for (auto& subCategory : category.getSubCategories()) {
			ImGui::Separator();
			renderCategory(*subCategory);
		}

		ImGui::TreePop();
	}
}

}

void SettingsWindow::_render() {
	for (auto& category : App::settings) {
		renderCategory(*category);
		ImGui::Separator();
	}
}
