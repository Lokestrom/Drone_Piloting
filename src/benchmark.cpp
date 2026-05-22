#include "benchmark.hpp"

#include <assert.h>
#include <chrono>

using namespace benchmark;

void benchmark::createFile(std::string filename) {
	if (filename == "") {
		auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		filename = "benchmark_" + std::to_string(time);
	}
	file.open(LogFilePlacement + filename + ".txt");
	assert(file && "Failed to create benchmark file");
}

void benchmark::closeFile() {
	file.close();
}