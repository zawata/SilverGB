cmake_minimum_required (VERSION 3.15)

# set(CMAKE_FIND_DEBUG_MODE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 20)
set(VERBOSE TRUE)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
                CACHE STRING "")
    else()
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
                CACHE STRING "")
    endif()
endif()
message("Using Toolchain file at: ${CMAKE_TOOLCHAIN_FILE}")

option(BUILD_SDL_UI "Build the SDL UI" OFF)
option(BUILD_IMGUI_UI "Build the ImGui UI" ON)
option(BUILD_WITH_ASAN "Build with AddressSanitizer enabled" OFF)

project ("SilverGB")

#global packages
find_package(nowide CONFIG REQUIRED)

add_subdirectory("src")