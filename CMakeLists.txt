cmake_minimum_required (VERSION 3.13)

set(CMAKE_FIND_DEBUG_MODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(VERBOSE ON)

#TODO:
# switches for which executable to compile

project ("SilverGB")

add_subdirectory("src")