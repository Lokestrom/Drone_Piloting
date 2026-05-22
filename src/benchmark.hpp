#pragma once

#include <fstream>

namespace benchmark {

constexpr bool enabled = false;

constexpr long long warmupTimeSeconds = 5;
constexpr long long benchmarkTimeSeconds = 30;
constexpr long long runs = 1;

inline const char* LogFilePlacement = ASSET_DIR "../benchmarks/";
inline std::ofstream file;

// This is the sum of all frame times, used to calculate the average frame time at the end of the benchmark
inline double averageFrameTimeSum = 0;

void createFile(std::string filename = "");
void closeFile();

}