#include "droneSolver.hpp"
#include <chrono>

using namespace solver;

void solver::solver(Builder builder) noexcept {
	auto start = std::chrono::high_resolution_clock::now();

	while (std::chrono::high_resolution_clock::now() < start + std::chrono::seconds(builder.time)) {
		builder.drone.update();
	}

}
