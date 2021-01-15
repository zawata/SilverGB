add_executable(gb_main_sdl
		"main.cpp")

find_package(SDL2 2.0.12 CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)

# target_include_directories(gb_main_sdl
# 	PRIVATE "./libnop/include")

target_link_libraries(gb_main_sdl
		gb_core
		util
		unofficial::gl3w::gl3w
		SDL2::SDL2
		SDL2::SDL2main
		${wxWidgets_LIBRARIES})