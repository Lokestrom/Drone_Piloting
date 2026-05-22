#include "importJSONData.hpp"

#include <glm/gtc/quaternion.hpp>

glm::vec3 getVec3(const Json& jsonObj) {
	return glm::vec3(jsonObj[0], jsonObj[1], jsonObj[2]);
}

glm::quat getQuat(const Json& jsonObj) {
	return glm::quat(glm::vec3(
		glm::radians(static_cast<float>(jsonObj[0])),
		glm::radians(static_cast<float>(jsonObj[1])),
		glm::radians(static_cast<float>(jsonObj[2]))));
}

glm::vec4 getVec4(const Json& jsonObj) {
	return glm::vec4{
		jsonObj[0],
		jsonObj[1],
		jsonObj[2],
		jsonObj[3],
	};
}

glm::mat3 getMat3(const Json& jsonObj) {
	return glm::mat3{
		jsonObj[0][0],
		jsonObj[0][1],
		jsonObj[0][2],
		jsonObj[1][0],
		jsonObj[1][1],
		jsonObj[1][2],
		jsonObj[2][0],
		jsonObj[2][1],
		jsonObj[2][2],
	};
}
