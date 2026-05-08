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

if(WIN32)
    if(DEFINED ENV{VULKAN_SDK})
        set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
        set(VULKAN_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include")
        set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/Lib")

        add_library(vulkan STATIC IMPORTED GLOBAL)
        set_target_properties(vulkan PROPERTIES
            IMPORTED_LOCATION "${VULKAN_LIB_DIR}/vulkan-1.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIR}"
        )
    else()
        message(WARNING "VULKAN_SDK not found! Install the Vulkan SDK, if you have done so already, make sure the environment variable VULKAN_SDK is set.")
    endif()
else()
    find_package(Vulkan REQUIRED)
endif()

target_include_directories(glfw INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glfw/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_include_directories(external INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json/single_include/nlohmann)
target_include_directories(external INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(external INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/eigen)
