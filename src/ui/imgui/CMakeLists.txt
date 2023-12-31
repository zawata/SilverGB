add_executable(gb_main_imgui)
target_sources(gb_main_imgui PRIVATE
    "app.cpp"
    "gui.cpp"
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

if(APPLE)
    target_sources(gb_main_imgui PRIVATE
        "platform/cocoa/main_macos.mm"
        "platform/cocoa/apple_menu.mm"
        "platform/cocoa/app_delegate.mm"
        "platform/cocoa/app_view_controller.mm"
        "platform/cocoa/callback_list.mm"
        "imgui/backends/imgui_impl_metal.mm"
        "imgui/backends/imgui_impl_osx.mm")

    find_library(METAL Metal)
    find_library(FOUNDATION Foundation)
    find_library(QUARTZCORE QuartzCore)
    target_link_libraries(gb_main_imgui PRIVATE
        "-framework Metal"
        "-framework MetalKit"
        "-framework Foundation"
        "-framework GameController"
        "-framework AppKit"
        objc)
elseif (UNIX)
    target_sources(gb_main_imgui PRIVATE
        "platform/gtk/main.cpp"
        "platform/gtk/gtk_application.cpp"
        "platform/gtk/imgui_impl_gtkmm.cpp"
        "imgui/backends/imgui_impl_opengl3.cpp"
    )
    find_package(PkgConfig)
    pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

    target_link_libraries(gb_main_imgui PRIVATE PkgConfig::GTKMM_VARS)
elseif (WIN32)
    target_sources(gb_main_imgui
        PRIVATE main_win32.cpp)
endif()

find_package(argparse CONFIG REQUIRED)
find_path(DIRENT_INCLUDE_DIRS "dirent.h")
target_link_libraries(gb_main_imgui
    PRIVATE
        gb_core
        util
        argparse::argparse)