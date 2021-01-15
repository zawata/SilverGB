add_executable(gb_main_imgui
        "main.cpp"
        "gui.cpp"
        "imgui/imgui.cpp"
        "imgui/imgui_demo.cpp"
        "imgui/imgui_draw.cpp"
        "imgui/imgui_impl_opengl3.cpp"
        "imgui/imgui_impl_sdl.cpp"
        "imgui/imgui_tables.cpp"
        "imgui/imgui_widgets.cpp"
        "imgui/ImGuiFileBrowser.cpp")

find_package(SDL2 2.0.12 CONFIG REQUIRED)
find_package(gl3w CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_path(DIRENT_INCLUDE_DIRS "dirent.h")
target_link_libraries(gb_main_imgui
    PRIVATE
        gb_core
        util
        SDL2::SDL2
        unofficial::gl3w::gl3w
        argparse::argparse
        ${DIRENT_INCLUDE_DIRS})