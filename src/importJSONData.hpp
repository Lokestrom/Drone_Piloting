#pragma once

#include <json.hpp>
#include <glm/glm.hpp>

using Json = nlohmann::json;

[[nodiscard]]
bool isNumber(const Json& jsonObj) noexcept;
[[nodiscard]]
bool isString(const Json& jsonObj) noexcept;
[[nodiscard]]
bool isArray(const Json& jsonObj) noexcept;
[[nodiscard]]
bool isVec3(const Json& jsonObj) noexcept;
[[nodiscard]]
bool isQuat(const Json& jsonObj) noexcept;
[[nodiscard]]
bool isVec4(const Json& jsonObj) noexcept;
[[nodiscard]]
bool isMat3(const Json& jsonObj) noexcept;

[[nodiscard]]
glm::vec3 getVec3(const Json& jsonObj) noexcept;
[[nodiscard]]
glm::quat getQuat(const Json& jsonObj) noexcept;
[[nodiscard]]
glm::vec4 getVec4(const Json& jsonObj) noexcept;
[[nodiscard]]
glm::mat3 getMat3(const Json& jsonObj) noexcept;