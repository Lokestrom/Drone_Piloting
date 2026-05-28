#pragma once

#include <json.hpp>
#include <glm/glm.hpp>

using Json = nlohmann::json;

bool isNumber(const Json& jsonObj);
bool isString(const Json& jsonObj);
bool isArray(const Json& jsonObj);
bool isVec3(const Json& jsonObj);
bool isQuat(const Json& jsonObj);
bool isVec4(const Json& jsonObj);
bool isMat3(const Json& jsonObj);

glm::vec3 getVec3(const Json& jsonObj);
glm::quat getQuat(const Json& jsonObj);
glm::vec4 getVec4(const Json& jsonObj);
glm::mat3 getMat3(const Json& jsonObj);