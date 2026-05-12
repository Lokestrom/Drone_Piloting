#include "console.hpp"

#include "../console.hpp"
#include "../gui/settingsGui.hpp"

void createConsoleSettings() {
	auto& settings = settings::Settings::newCategory("Console");

	settings.emplace<settings::Value<glm::vec3>>("Message color", 
		glm::vec3{ 1, 1, 1 }, settings::Value<glm::vec3>::setFunctionT(gui::color));
	settings.emplace<settings::Value<glm::vec3>>("Warning color",
		glm::vec3{ 1, 1, 0.2 }, settings::Value<glm::vec3>::setFunctionT(gui::color));
	settings.emplace<settings::Value<glm::vec3>>("Error color",
		glm::vec3{ 1, 0.1, 0.1 }, settings::Value<glm::vec3>::setFunctionT(gui::color));
}

ConsoleWindow::ConsoleWindow() noexcept
	: gui::ImGuiWindow("Console", true) 
	, _messageColor(settings::Settings::get("Console").get<glm::vec3>("Message color").getHandle()) 
	, _warningColor(settings::Settings::get("Console").get<glm::vec3>("Warning color").getHandle())
	, _errorColor(settings::Settings::get("Console").get<glm::vec3>("Error color").getHandle())
	, _showOld(false)
{ }

namespace {
ImColor toColor(glm::vec3 v) {
	return ImColor{ v.x, v.y, v.z };
}

}

void ConsoleWindow::print(const Console::Log& log) {
	ImColor color{ 0, 0, 0 };
	switch (log.type) {
	case Console::Type::meassage:
		color = toColor(_messageColor.get());
		break;
	case Console::Type::warning:
	case Console::Type::debug:
		color = toColor(_warningColor.get());
		break;
	case Console::Type::error:
		color = toColor(_errorColor.get());
		break;
	default:
		assert(false && "One log type is unaccounted for");
	}


	ImGui::TextColored(color, log.what.c_str());
}

void ConsoleWindow::render() {
	begin();

	if (_showOld) {
		for (size_t i = 0; i < _clearIndex; i++) {
			print(Console::getLogs()[i]);
		}
		ImGui::Text("Old");
		ImGui::Separator();
		ImGui::Text("New");
	}

	for (size_t i = _clearIndex; i < Console::getLogs().size(); i++) {
		print(Console::getLogs()[i]);
	}

	if (ImGui::BeginPopupContextWindow("ConsoleContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Clear")) {
			_clearIndex = Console::getLogs().size();
		}

		if (ImGui::MenuItem("Show Old")) {
			_showOld = !_showOld;
		}

		ImGui::EndPopup();
	}


	end();
}