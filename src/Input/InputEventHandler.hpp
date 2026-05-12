#pragma once

#include <glm/glm.hpp>

class InputEventHandler {
public:
	static inline double mouseScrollWheel;
	
	static void reset() noexcept { mouseScrollWheel = 0; }

	static inline glm::vec<2, double> lastMousePos;
	static inline glm::vec<2, double> mouseDelta;
};