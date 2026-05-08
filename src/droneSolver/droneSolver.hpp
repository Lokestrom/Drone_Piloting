#pragma once

#include "../Drone.hpp"

namespace solver {

struct Input {
	UserInput input;
	float time;
	float length;
};

struct Builder {
	float dt;
	float time;
	Drone& drone;
	// map sandbox
	std::vector<Input> inputs;
};

void solver(Builder builder) noexcept;

}