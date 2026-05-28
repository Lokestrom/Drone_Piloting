#pragma once

#include "rendering/gameObject.hpp"
#include "importJSONData.hpp"

#include <filesystem>

// split in 2 parts
// create a sandbox with no objects just the forces
// so that it can be used in the droneSolver

class Map {
public:
	Map() noexcept = default;
	
	Map(Map&) = delete;
	Map& operator=(Map&) = delete;

	Map(Map&&) noexcept = default;
	Map& operator=(Map&&) noexcept = default;
	~Map() noexcept;

	[[nodiscard]]
	bool load(std::filesystem::path folderPath);
	void unload();

	const glm::vec3& getLightSourcePos() const noexcept { return lightSourcePos; }
private:
	// prints every error it can find before returning if it is valid or not
	bool verifyMapFolder(std::filesystem::path folderPath) const;
	bool verifyConfigFile(Json jsonData, std::filesystem::path folderPath) const;

	std::vector<vulkan::ID> sceneryIDs;
	glm::vec3 lightSourcePos = glm::vec3(0,0,0);
};