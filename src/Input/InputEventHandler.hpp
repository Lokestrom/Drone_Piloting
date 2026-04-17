#pragma once

class InputEventHandler {
public:
	static inline double mouseScrollWheel;
	
	static void reset() noexcept { mouseScrollWheel = 0; }
};