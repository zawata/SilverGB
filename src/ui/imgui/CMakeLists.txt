add_executable(gb_main_imgui)
target_sources(gb_main_imgui PRIVATE
        "app.cpp"
        "gui.cpp"
        "../common/binding.cpp"
        "imgui/imgui.cpp"
        "imgui/imgui_demo.cpp"
        "imgui/imgui_draw.cpp"
        "imgui/imgui_tables.cpp"
        "imgui/imgui_widgets.cpp")
target_include_directories(gb_main_imgui PRIVATE
        "./"
        "../common"
        "imgui"
        "imgui/backends"
        "platform")

# TODO: I don't know if this will work on windows
find_package(PkgConfig)
find_package(portaudio CONFIG REQUIRED)
find_path(LIBNOP_INCLUDE_DIRS "nop/base/array.h")

target_include_directories(gb_main_imgui
        PRIVATE ${LIBNOP_INCLUDE_DIRS})
target_link_libraries(gb_main_imgui PRIVATE
        portaudio_static)

if (APPLE)
    target_compile_definitions(gb_main_imgui PRIVATE
            "-DUSE_SDLINPUT")

    target_sources(gb_main_imgui PRIVATE
            "../common/input/gamepad_sdlinput.cpp"
            "platform/cocoa/main_macos.mm"
            "platform/cocoa/apple_menu.mm"
            "platform/cocoa/app_delegate.mm"
            "platform/cocoa/app_view_controller.mm"
            "platform/cocoa/callback_list.mm"
            "imgui/backends/imgui_impl_metal.mm"
            "imgui/backends/imgui_impl_osx.mm")

    find_package(SDL2 CONFIG REQUIRED)

    find_library(METAL Metal)
    find_library(FOUNDATION Foundation)
    find_library(QUARTZCORE QuartzCore)
    target_link_libraries(gb_main_imgui PRIVATE
            "-framework Metal"
            "-framework MetalKit"
            "-framework Foundation"
            # "-framework GameController"
            "-framework AppKit"
            objc
            SDL2::SDL2)
elseif (UNIX)
    target_compile_definitions(gb_main_imgui PRIVATE
            "-DUSE_SDLINPUT")

    target_sources(gb_main_imgui PRIVATE
            "../common/input/gamepad_sdlinput.cpp"
            "platform/gtk/main.cpp"
            "platform/gtk/gtk_application.cpp"
            "platform/gtk/imgui_impl_gtkmm.cpp"
            "platform/gtk/textures.cpp"
            "imgui/backends/imgui_impl_opengl3.cpp"
    )

    pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

    find_package(SDL2 CONFIG REQUIRED)

    target_link_libraries(gb_main_imgui PRIVATE
            PkgConfig::GTKMM_VARS
            SDL2::SDL2
    )
elseif (WIN32)
    target_sources(gb_main_imgui
            PRIVATE main_win32.cpp)
endif ()

find_package(argparse CONFIG REQUIRED)
find_path(DIRENT_INCLUDE_DIRS "dirent.h")
target_link_libraries(gb_main_imgui
        PRIVATE
        gb_core
        util
        argparse::argparse)