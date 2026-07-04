#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <glm/vec3.hpp>

namespace benchmark {

inline constexpr bool enabled = false;
inline constexpr std::string_view configFileName = "default.json";
inline constexpr std::string_view runName = "benchmark3";

struct Config {
	size_t warmupTimeSeconds;
	size_t benchmarkTimeSeconds;
	size_t runs;
	std::filesystem::path map;
	std::filesystem::path drone;
	glm::vec3 dronePosition;
};

void initialize();
[[nodiscard]] const Config& getConfig();
void beginRun(size_t run);
void recordStartupTime(size_t milliseconds);
void recordShutdownTime(size_t milliseconds);
void recordAverageFrameTime(double milliseconds);
void finalize();

}
