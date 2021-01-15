#includes
include_directories("./inc")

#core files
add_subdirectory("./gb_core")
add_subdirectory("./util")

#interfaces
add_subdirectory("./ui/wx")
add_subdirectory("./ui/sdl")
# add_subdirectory("./ui/imgui")

