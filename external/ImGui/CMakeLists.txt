cmake_minimum_required (VERSION 3.10)

file(GLOB CPP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(imgui STATIC ${CPP_SOURCES})

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
