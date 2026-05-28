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
ImColor toColor(glm::vec3 v) noexcept {
	return ImColor{ v.x, v.y, v.z };
}

}

void ConsoleWindow::print(const Console::Log& log) {
	ImColor color{ 0, 0, 0 };
	std::string prefix;
	switch (log.type) {
	case Console::Log::Type::message:
		if (!_showMessages)
			return;
		color = toColor(_messageColor.get());
		prefix = "Message";
		break;
	case Console::Log::Type::warning:
		if (!_showWarnings)
			return;
		color = toColor(_warningColor.get());
		prefix = "Warning";
		break;
	case Console::Log::Type::debug:
		color = toColor(_warningColor.get());
		prefix = "Debug";
		break;
	case Console::Log::Type::error:
		if (!_showErrors)
			return;
		color = toColor(_errorColor.get());
		prefix = "Error";
		break;
	default:
		assert(false && "Log type is unaccounted for");
	}
	prefix += ": ";

	ImGui::TextColored(color, (prefix + log.what).c_str());
}

void ConsoleWindow::_render() {
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

		if (ImGui::MenuItem("Show Old", nullptr, _showOld)) {
			_showOld = !_showOld;
		}
		if (ImGui::MenuItem("Show Messages", nullptr, _showMessages)) {
			_showMessages = !_showMessages;
		}
		if (ImGui::MenuItem("Show Warnings", nullptr, _showWarnings)) {
			_showWarnings = !_showWarnings;
		}
		if (ImGui::MenuItem("Show Errors", nullptr, _showErrors)) {
			_showErrors = !_showErrors;
		}
		ImGui::EndPopup();
	}
}