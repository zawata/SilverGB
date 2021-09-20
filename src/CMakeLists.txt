#includes
include_directories("./inc")

add_subdirectory("./util")

#core files
add_subdirectory("./gb_core")
add_subdirectory("./gba_core")

#interfaces
add_subdirectory("./ui/wx")
add_subdirectory("./ui/sdl")
# add_subdirectory("./ui/imgui")

#TODO: remove
add_executable(gba_main
    "gba_core/main.cpp")

target_link_libraries(gba_main
    gba_core
    util)