cmake_minimum_required (VERSION 3.15)

# set(CMAKE_FIND_DEBUG_MODE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 17)
# set(VERBOSE TRUE)

# UIs
option(BUILD_WX_UI "Build the wxWidgets UI" OFF)
option(BUILD_SDL_UI "Build the SDL UI" OFF)
option(BUILD_IMGUI_UI "Build the ImGui UI" OFF)

# librarys
option(BUILD_WASM_MOD "Build a webassembly Module(overrides all UI options)" OFF)

# debugging options
# TODO: maybe always set this in debug?
option(BUILD_WITH_ASAN "Build with AddressSanitizer enabled" OFF)

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

if(BUILD_WASM_MOD)
    # override all the UI targets
    set(BUILD_WX_UI OFF)
    set(BUILD_SDL_UI OFF)
    set(BUILD_IMGUI_UI OFF)

    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "wasm32-emscripten" CACHE STRING "")
    endif()

    # emscripten requires a specific environment to run correctly. since I really
    # don't want to have to call the wrapper script I'll jst do it myself
    # https://github.com/emscripten-core/emscripten/blob/main/emcmake.py

    # TODO: we need to get the emscripten dir programmatically. Ideas:
    # - it's on the path so we can try to look up the location of emcc or emcmake
    #   but I'm not sure how to do that platform independently
    # - EM_CONFIG contains the location of the config file which as EMSCRIPTEN_ROOT defined in it
    #   but this requires importing a python module that violates like 3 naming conventions
    # - I'm wondering if we could also just call to the emsdk binary and get it to spit the path out

    set(EMSCRIPTEN_ROOT "$ENV{EMSDK}/upstream/emscripten")

    # TODO: bail if we're on windows and using the VS generator

    if(NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
        set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake"
                CACHE STRING "")
    endif()
    message("Chainloading Toolchain file at: ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")

    # emcmake also sets the CMAKE_CROSSCOMPILING_EMULATOR variable but I'm pretty sure that we don't need it
    # also apparently emscripten ships with it's own node binary which is likely what this is set to
endif()

project ("SilverGB")

#global packages
find_package(nowide CONFIG REQUIRED)

add_subdirectory("src")