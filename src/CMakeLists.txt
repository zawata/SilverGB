﻿#includes
include_directories("../res")

#core files
add_subdirectory("./gb_core")
# add_subdirectory("./gba_core")
add_subdirectory("./util")

if(BUILD_SDL_UI)
    add_subdirectory("./ui/sdl")
endif()

if(BUILD_IMGUI_UI)
    add_subdirectory("./ui/imgui")
endif()