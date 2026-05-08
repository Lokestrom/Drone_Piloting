#include "../DroneAPI.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <vector>


struct DroneEngine {
	size_t id;
	glm::vec3 position;
	glm::vec3 forceDirection;
	float maxThrust;
};

struct Drone {
	float mass;
	glm::mat3 inertiaTensor;
	std::vector<DroneEngine> engines;
};

Drone getDrone(const char* dronePath);