#include "benchmark.hpp"

#include <json.hpp>

#include <array>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "importJSONData.hpp"

using namespace benchmark;
using Json = nlohmann::json;

namespace {

Config config;
std::filesystem::path configPath;
std::ofstream file;
size_t totalStartupTime = 0;
size_t totalShutdownTime = 0;
double averageFrameTimeSum = 0;

[[nodiscard]]
size_t getUnsignedInteger(const Json& json, const char* field, bool allowZero) {
	if (!json.contains(field) || !isUnsignedInteger(json[field])) {
		throw std::runtime_error(std::string("Benchmark config field '") + field + "' must be an unsigned integer");
	}
	const size_t value = json[field].get<size_t>();
	if (!allowZero && value == 0) {
		throw std::runtime_error(
			std::string("Benchmark config field '") + field +
			(allowZero ? "' must not be negative" : "' must be greater than zero"));
	}
	return value;
}

[[nodiscard]]
std::filesystem::path getAssetPath(
	const Json& json,
	const char* field,
	const char* relativeAssetDirectory) {
	if (!json.contains(field) || !json[field].is_string()) {
		throw std::runtime_error(std::string("Benchmark config field '") + field + "' must be a string");
	}

	std::filesystem::path value = json[field].get<std::string>();
	if (value.empty()) {
		throw std::runtime_error(
			std::string("Benchmark config field '") + field + "' must not be empty");
	}
	if (!value.is_absolute()) {
		value = std::filesystem::path(ASSET_DIR) / relativeAssetDirectory / value;
	}
	value = value.lexically_normal();

	if (!std::filesystem::is_directory(value)) {
		throw std::runtime_error(
			std::string("Benchmark config field '") + field +
			"' does not point to a directory: " + value.string());
	}
	return value;
}

[[nodiscard]]
glm::vec3 getPosition(const Json& json) {
	constexpr const char* field = "dronePosition";
	if (!json.contains(field) || !isVec3(json[field])) {
		throw std::runtime_error("Benchmark config field 'dronePosition' must be an array of three numbers");
	}
	return getVec3(json[field]);
}

void createFile() {
	const std::filesystem::path outputDirectory = BENCHMARK_DIR;
	const std::filesystem::path requestedRunName = runName;
	if (requestedRunName.empty() ||
		requestedRunName.is_absolute() ||
		requestedRunName.has_parent_path()) {
		throw std::runtime_error("Benchmark run name must be a file name");
	}

	std::string filename = requestedRunName.string();
	if (std::filesystem::exists(outputDirectory / (filename + ".txt"))) {
		int i = 1;
		while (std::filesystem::exists(outputDirectory / (filename + "_" + std::to_string(i) + ".txt"))) {
			i++;
		}
		filename += "_" + std::to_string(i);
	}

	file.open(outputDirectory / (filename + ".txt"));
	if (!file) {
		throw std::runtime_error(
			"Failed to create benchmark output file: " +
			(outputDirectory / (filename + ".txt")).string());
	}
}

}

void benchmark::initialize() {
	assert(enabled && "Benchmark config must only be initialized in a benchmark build");
	totalStartupTime = 0;
	totalShutdownTime = 0;
	averageFrameTimeSum = 0;

	std::filesystem::path requestedConfig = configFileName;
	if (requestedConfig.empty() || requestedConfig.is_absolute() || requestedConfig.has_parent_path()) {
		throw std::runtime_error("Benchmark config must be a file name from benchmarks/configs");
	}
	if (requestedConfig.extension() != ".json") {
		throw std::runtime_error("Benchmark config must use the .json extension");
	}

	configPath = std::filesystem::path(BENCHMARK_DIR) / "configs" / requestedConfig;
	std::ifstream configFile(configPath);
	if (!configFile) {
		throw std::runtime_error("Failed to open benchmark config: " + configPath.string());
	}

	Json json;
	try {
		json = Json::parse(configFile, nullptr, true, true);
	}
	catch (const std::exception& e) {
		throw std::runtime_error(
			"Failed to parse benchmark config '" + configPath.string() + "': " + e.what());
	}

	config = Config{
		.warmupTimeSeconds = getUnsignedInteger(json, "warmupTimeSeconds", true),
		.benchmarkTimeSeconds = getUnsignedInteger(json, "benchmarkTimeSeconds", false),
		.runs = getUnsignedInteger(json, "runs", false),
		.map = getAssetPath(json, "map", "Maps"),
		.drone = getAssetPath(json, "drone", "Drones"),
		.dronePosition = getPosition(json)
	};

	createFile();
	file << "Run name: " << runName << "\n";
	file << "Config: " << configPath.filename().string() << "\n";
	file << "Map: " << config.map.string() << "\n";
	file << "Drone: " << config.drone.string() << "\n";
	file << "Drone position: ["
		<< config.dronePosition[0] << ", "
		<< config.dronePosition[1] << ", "
		<< config.dronePosition[2] << "]\n";
	file << "Benchmarking with " << config.runs << " runs\n";
}

const Config& benchmark::getConfig() {
	assert(enabled && "A benchmark config is only available in a benchmark build");
	return config;
}

void benchmark::beginRun(size_t run) {
	assert(run < config.runs && "Benchmark run index is out of range");
	file << "\n\n\nRun " << run + 1 << "/" << config.runs << "\n";
}

void benchmark::recordStartupTime(size_t milliseconds) {
	file << "Startup time: " << milliseconds << " ms\n";
	totalStartupTime += milliseconds;
}

void benchmark::recordShutdownTime(size_t milliseconds) {
	file << "Shutdown time: " << milliseconds << " ms\n";
	totalShutdownTime += milliseconds;
}

void benchmark::recordAverageFrameTime(double milliseconds) {
	file << "Average frame time: " << milliseconds << " ms\n";
	averageFrameTimeSum += milliseconds;
}

void benchmark::finalize() {
	file << "\n\n\nSummary:\n";
	file << "Total startup time: " << totalStartupTime << " ms\n";
	file << "Average startup time: " << totalStartupTime / static_cast<double>(config.runs) << " ms\n";
	file << "Total shutdown time: " << totalShutdownTime << " ms\n";
	file << "Average shutdown time: " << totalShutdownTime / static_cast<double>(config.runs) << " ms\n";
	file << "\nAverage frame time: " << averageFrameTimeSum / static_cast<double>(config.runs) << " ms\n";
	file.close();
}
