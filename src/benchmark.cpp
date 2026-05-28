#include "benchmark.hpp"

#include <assert.h>
#include <chrono>
#include <filesystem>

using namespace benchmark;

void benchmark::createFile(std::string filename) {
	if (filename == "") {
		auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		filename = "benchmark_" + std::to_string(time);
	}
	if (std::filesystem::exists(LogFilePlacement + filename + ".txt")) {
		int i = 1;
		while (std::filesystem::exists(LogFilePlacement + filename + "_" + std::to_string(i) + ".txt")) {
			i++;
		}
		filename += "_" + std::to_string(i);
	}
	file.open(LogFilePlacement + filename + ".txt");
	assert(file && "Failed to create benchmark file");
}

void benchmark::closeFile() {
	file.close();
}