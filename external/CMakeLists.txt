cmake_minimum_required(VERSION 4.1)

message(STATUS "Configuring external dependencies")

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_library(external INTERFACE)

add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(json)
add_subdirectory(ImGui)
add_subdirectory(eigen)

target_link_libraries(external INTERFACE glfw glm nlohmann_json::nlohmann_json)

target_include_directories(glfw INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glfw/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_include_directories(external INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json/single_include/nlohmann)
target_include_directories(external INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(external INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/eigen)
