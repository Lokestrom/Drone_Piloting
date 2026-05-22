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

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic)
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /permissive-)
endif()

add_executable(app ${cppFiles})
target_link_libraries(app PUBLIC imgui external vulkanInterface guiInterface windows)
target_include_directories(app PUBLIC ${CMAKE_SOURCE_DIR})
