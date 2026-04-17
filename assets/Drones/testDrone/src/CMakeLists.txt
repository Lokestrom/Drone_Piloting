cmake_minimum_required (VERSION 4.1)

project ("Test Drone")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB cppFiles
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(control SHARED ${cppFiles})

set(DRONE_ROOT "${CMAKE_SOURCE_DIR}/..")

set_target_properties(control PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${DRONE_ROOT}"
    LIBRARY_OUTPUT_DIRECTORY "${DRONE_ROOT}"
    ARCHIVE_OUTPUT_DIRECTORY "${DRONE_ROOT}"
)