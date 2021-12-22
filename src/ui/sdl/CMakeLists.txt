find_package(SDL2 CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(RtAudio CONFIG REQUIRED)

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
		"../common/audio/audio_rtaudio.cpp")
target_include_directories(gb_main_opengl
		PRIVATE "../common/audio/")
target_link_libraries(gb_main_opengl
		gb_core
		util
		RtAudio::rtaudio
		nowide::nowide
		SDL2::SDL2
		SDL2::SDL2main
		unofficial::gl3w::gl3w)