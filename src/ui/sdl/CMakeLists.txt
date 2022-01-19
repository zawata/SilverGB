find_package(SDL2 CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(portaudio CONFIG REQUIRED)

add_executable(gb_main_sdl
		"main_sdl.cpp"
		"../common/audio/audio_sdlaudio.cpp")
target_include_directories(gb_main_sdl
		PRIVATE "../common/audio/")
target_link_libraries(gb_main_sdl
		gb_core
		util
		nowide::nowide
		SDL2::SDL2
		SDL2::SDL2main)

add_executable(gb_main_opengl
		"main_opengl.cpp"
		"../common/audio/audio_portaudio.cpp")
target_include_directories(gb_main_opengl
		PRIVATE "../common/audio/")
target_link_libraries(gb_main_opengl
		gb_core
		util
		portaudio_static
		nowide::nowide
		SDL2::SDL2
		SDL2::SDL2main
		unofficial::gl3w::gl3w)

# Fix for wayland>=1.20 and SDL2 < 2.0.20, remove once sdl fixes this
find_package(PkgConfig)
if(SDL2_VERSION VERSION_LESS "2.0.20" AND PKG_CONFIG_FOUND)
	pkg_check_modules(WAYLAND_CLIENT wayland-client>=1.20.0)
	if(WAYLAND_CLIENT_FOUND)
		target_link_libraries(gb_main_sdl
			${WAYLAND_CLIENT_LIBRARIES})
		target_link_libraries(gb_main_opengl
			${WAYLAND_CLIENT_LIBRARIES})
	endif()
endif()