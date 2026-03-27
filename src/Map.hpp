#pragma once

#include "rendering/gameObject.hpp"

#include <filesystem>

class Map {
public:
	Map() noexcept = default;
	
	Map(Map&) = delete;
	Map& operator=(Map&) = delete;

	Map(Map&&) noexcept = default;
	Map& operator=(Map&&) noexcept = default;

	Map(std::filesystem::path folderPath);

	~Map() noexcept;

	void load(std::filesystem::path folderPath);
	void unload();

	const glm::vec3& getLightSourcePos() const noexcept { return lightSourcePos; }

private:
	std::vector<vulkan::ID> sceneryIDs;
	glm::vec3 lightSourcePos = glm::vec3(0,0,0);
};