#pragma once

#include <string_view>

namespace gui {

class ImGuiWindow {
public:
	ImGuiWindow(std::string_view name, bool open) noexcept;
	virtual ~ImGuiWindow() = default;

	virtual void render();

	bool isOpen() const noexcept { return _open; }
	const std::string_view& getName() const noexcept { return _name; }

	void open() noexcept { _open = true;  }

protected:
	void begin();
	void end();

private:

	std::string_view _name;
	bool _open;
};
}