#pragma once

#include <json.hpp>
#include <glm/glm.hpp>

using Json = nlohmann::json;

glm::vec3 getVec3(const Json& jsonObj);
glm::quat getQuat(const Json& jsonObj);

glm::vec4 getVec4(const Json& jsonObj);

glm::mat3 getMat3(const Json& jsonObj);