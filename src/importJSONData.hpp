#pragma once

#include <json.hpp>
#include <glm/glm.hpp>

using Json = nlohmann::json;

[[nodiscard]]
bool isNumber(const Json& jsonObj);
[[nodiscard]]
bool isString(const Json& jsonObj);
[[nodiscard]]
bool isArray(const Json& jsonObj);
[[nodiscard]]
bool isVec3(const Json& jsonObj);
[[nodiscard]]
bool isQuat(const Json& jsonObj);
[[nodiscard]]
bool isVec4(const Json& jsonObj);
[[nodiscard]]
bool isMat3(const Json& jsonObj);

[[nodiscard]]
glm::vec3 getVec3(const Json& jsonObj);
[[nodiscard]]
glm::quat getQuat(const Json& jsonObj);
[[nodiscard]]
glm::vec4 getVec4(const Json& jsonObj);
[[nodiscard]]
glm::mat3 getMat3(const Json& jsonObj);