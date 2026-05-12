cmake_minimum_required (VERSION 4.1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB cppFiles
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_subdirectory(rendering)
add_subdirectory(windows)
add_subdirectory(gui)
add_subdirectory(shaders)

add_executable(app WIN32 ${cppFiles})
target_link_libraries(app PUBLIC imgui external vulkanInterface guiInterface windows)
target_include_directories(app PUBLIC ${CMAKE_SOURCE_DIR})
