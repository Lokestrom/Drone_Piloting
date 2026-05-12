#pragma once

#include "../gui/window.hpp"

#include "../Settings.hpp"
#include <glm/glm/vec3.hpp>
#include "../console.hpp"

void createConsoleSettings();

class ConsoleWindow
	: public gui::ImGuiWindow {
public:
	ConsoleWindow() noexcept;

	void render() override;

private:

	void print(const Console::Log& log);

private:
	static inline size_t _clearIndex = 0;
	bool _showOld;
	settings::ValueHandle<glm::vec3> _messageColor;
	settings::ValueHandle<glm::vec3> _warningColor;
	settings::ValueHandle<glm::vec3> _errorColor;
};