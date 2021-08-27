add_executable(gb_main_sdl
		"main.cpp")

find_package(SDL2 CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)

# target_include_directories(gb_main_sdl
# 	PRIVATE "./libnop/include")

target_link_libraries(gb_main_sdl
		gb_core
		util
		SDL2::SDL2
		SDL2::SDL2main
		unofficial::gl3w::gl3w)