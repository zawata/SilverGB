#pragma once

#include <list>

#ifdef _IS_WINDOWS
#  include <functional>
#endif

namespace NUI {

typedef std::function<void()>              event_callback_t;
typedef std::function<void()>             draw_callback_t;
typedef std::function<void(const int, const std::string &)> menu_callback_t;

struct MenuBar;

struct Window {
    static const int DefaultWidth = 800;
    static const int DefaultHeight = 600;

    virtual void set_title(const std::string &s) = 0;
    virtual bool add_menubar(MenuBar *n) = 0;

    virtual void set_position(int x, int y) = 0;
    virtual void set_dimensions(int w, int h) = 0;

    virtual int run_loop() = 0;

    virtual void set_on_event_callback(event_callback_t) = 0;
    virtual void set_on_draw_callback(draw_callback_t) = 0;
    virtual void set_on_menu_callback(menu_callback_t) = 0;

    virtual void message_box(std::string &, std::string &) = 0;

protected:
    Window() {
        on_event_callback = on_event_default_callback;
        on_draw_callback = on_draw_default_callback;
        on_menu_callback = on_menu_default_callback;
    }

    virtual ~Window() {}

    event_callback_t on_event_callback;
    draw_callback_t  on_draw_callback;
    menu_callback_t  on_menu_callback;

    event_callback_t on_event_default_callback = []() -> void {
        return;
    };

    draw_callback_t on_draw_default_callback = []() -> void {
        return;
    };

    menu_callback_t on_menu_default_callback = [](const int, const std::string &) -> void {
        return;
    };
};

extern Window *CreateWindow(const std::string &s, int x = Window::DefaultWidth, int y = Window::DefaultHeight);

} // namespace NUI;

NUI::Window *userMain(std::list<std::string> args);