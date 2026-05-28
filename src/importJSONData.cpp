#include "importJSONData.hpp"

#include <glm/gtc/quaternion.hpp>

bool isNumber(const Json& jsonObj) {
	return jsonObj.is_number();
}
bool isString(const Json& jsonObj) {
	return jsonObj.is_string();
}
bool isArray(const Json& jsonObj) {
	return jsonObj.is_array();
}
bool isVec3(const Json& jsonObj) {
	return isArray(jsonObj) && jsonObj.size() == 3 && isNumber(jsonObj[0]) && isNumber(jsonObj[1]) && isNumber(jsonObj[2]);
}
// quats start as euler angles in degrees and are converted to quats when read in, so they have the same format as vec3s
bool isQuat(const Json& jsonObj) {
	return isVec3(jsonObj);
}
bool isVec4(const Json& jsonObj) {
	return isArray(jsonObj) && jsonObj.size() == 4 && isNumber(jsonObj[0]) && isNumber(jsonObj[1]) && isNumber(jsonObj[2]) && isNumber(jsonObj[3]);
}
bool isMat3(const Json& jsonObj) {
	return isArray(jsonObj) && jsonObj.size() == 3 && isVec3(jsonObj[0]) && isVec3(jsonObj[1]) && isVec3(jsonObj[2]);
}

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
