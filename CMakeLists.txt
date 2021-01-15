cmake_minimum_required (VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "cmake")
# set(CMAKE_FIND_DEBUG_MODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(VERBOSE ON)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

#TODO:
# switches for which executable to compile

project ("SilverGB")

add_subdirectory("src")