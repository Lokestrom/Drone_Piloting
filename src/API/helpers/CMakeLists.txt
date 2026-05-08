cmake_minimum_required (VERSION 4.1)

project("Drone piloter API helpers")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB cppFiles
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(DronePiloterAPI ${cppFiles})
target_link_libraries(DronePiloterAPI PUBLIC external)