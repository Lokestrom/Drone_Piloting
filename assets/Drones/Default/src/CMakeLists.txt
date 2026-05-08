cmake_minimum_required (VERSION 4.1)

project ("Default Drone")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB cppFiles
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(defaultDrone SHARED ${cppFiles})

set(DRONE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

set_target_properties(defaultDrone PROPERTIES
    OUTPUT_NAME "control"
    RUNTIME_OUTPUT_DIRECTORY "${DRONE_ROOT}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${DRONE_ROOT}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${DRONE_ROOT}/$<CONFIG>"
)

target_link_libraries(defaultDrone PUBLIC external DronePiloterAPI)