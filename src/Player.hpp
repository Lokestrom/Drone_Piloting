#include "Drone.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Renderer.hpp"

#include <optional>
#include <filesystem>

class Player {
public:
	Player() noexcept;
	Player(std::filesystem::path folderPath);

	Player(Player&) = delete;
	Player& operator=(Player&) = delete;

	Player(Player&&) noexcept = default;
	Player& operator=(Player&&) noexcept = delete;

	void SwapDrone(std::filesystem::path folderPath);
	void releaseDrone();

	vulkan::UniformBufferObject getUBO() const noexcept;

	void update();

	Camera& getCamera() noexcept { return _camera; }
	std::optional<Drone>& getDrone() noexcept { return _drone; }

private:
	std::optional<Drone> _drone;
	Camera _camera;
	bool freeCAM; // it the future this is when _drone does not have value
};