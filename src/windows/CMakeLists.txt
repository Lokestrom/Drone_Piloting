cmake_minimum_required (VERSION 4.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB cppFiles
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(windows ${cppFiles})

target_link_libraries(windows PUBLIC external guiInterface)

target_include_directories(windows PUBLIC ${CMAKE_SOURCE_DIR})