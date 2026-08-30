#pragma once

#include "rendering/gameObject.hpp"
#include "importJSONData.hpp"

#include <filesystem>
#include <string>

// split in 2 parts
// create a sandbox with no objects just the forces
// so that it can be used in the droneSolver

// may be useful in the future
//class MapConfig {
//public:
//	struct Object {
//		std::string model;
//		glm::vec3 position;
//		glm::quat rotation;
//		glm::vec3 scale;
//	};
//
//	MapConfig() noexcept;
//	MapConfig(std::filesystem::path folder, bool withObjects = true);
//
//	// There should never be a case where copying this is a good option
//	MapConfig(const MapConfig&) = delete;
//	MapConfig& operator=(const MapConfig&) = delete;
//
//	MapConfig(MapConfig&&) noexcept = default;
//	MapConfig& operator=(MapConfig&&) noexcept = default;
//	
//	const std::filesystem::path& folder() const noexcept;
//	const std::string& name() const noexcept;
//	const std::string& description() const noexcept;
//	const std::vector<Object>& objects() const noexcept;
//
//	bool isValid() const;
//
//private:
//	const std::filesystem::path _folder;
//	const std::string _name;
//	const std::string _description;
//
//	const std::vector<Object> _Objects;
//	// prints every error it can find before returning if it is valid or not
//	bool verifyMapFolder(std::filesystem::path folderPath) const;
//	bool verifyConfigFile(std::filesystem::path folderPath) const;
//};

class Map {
public:
	Map() noexcept = default;
	
	Map(Map&) = delete;
	Map& operator=(Map&) = delete;

	Map(Map&& other) noexcept;
	Map& operator=(Map&& other) noexcept;
	~Map() noexcept;

	[[nodiscard]]
	bool load(std::filesystem::path folderPath);
	void unload() noexcept;

	[[nodiscard]]
	const glm::vec3& getLightSourcePos() const noexcept { return lightSourcePos; }

private:
	// prints every error it can find before returning if it is valid or not
	[[nodiscard]]
	bool verifyMapFolder(std::filesystem::path folderPath) const;
	[[nodiscard]]
	bool verifyConfigFile(Json jsonData, std::filesystem::path folderPath) const;

	std::vector<renderer::ID> sceneryIDs;
	glm::vec3 lightSourcePos = glm::vec3(0,0,0);
};
