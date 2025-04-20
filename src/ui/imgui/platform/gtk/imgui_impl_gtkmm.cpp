#include "imgui_impl_gtkmm.hpp"

#include <chrono>
#include <cstring>
#include <gdkmm/enums.h>
#include <gdkmm/event.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/glarea.h>
#include <iostream>
#include <mutex>

#include "util/util.hpp"

#include "imgui.h"

struct ImGui_ImplGtkmm_Data {
    Gtk::ApplicationWindow                        *window;
    Gtk::GLArea                                   *glArea;
    std::chrono::high_resolution_clock::time_point last_frame_time;
};

static ImGui_ImplGtkmm_Data *ImGui_ImplGtkmm_GetBackendData() {
    return ImGui::GetCurrentContext() ? (ImGui_ImplGtkmm_Data *)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

static void ImGui_ImplGtkmm_SetClipboardText(void * /* userData */, const char *text) {
    ImGui_ImplGtkmm_Data *data = ImGui_ImplGtkmm_GetBackendData();

    data->glArea->get_clipboard()->set_text(text);
}

static const char *ImGui_ImplGtkmm_GetClipboardText(void * /* userData */) {
    ImGui_ImplGtkmm_Data *data    = ImGui_ImplGtkmm_GetBackendData();

    // TODO: this is probably a bad implementation
    // may need a global clipboard manager that asynchronously
    // buffers changes and we just copy from it
    auto                  content = data->glArea->get_clipboard()->get_content();
    if(content) {
        Glib::Value<std::string> v;
        content->get_property_value("text/plain", v);
        return strdup(v.get().c_str());
    }

    return "";
}

static void ImGui_ImplGtkmm_SetPlatformImeData(ImGuiViewport *, ImGuiPlatformImeData *data) {
    if(data->WantVisible) {
        // TODO: do something ?
    }
}

bool ImGui_ImplGtkmm_Init(Gtk::ApplicationWindow *window, Gtk::GLArea *glArea) {
    ImGuiIO &io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplGtkmm_Data *bd         = IM_NEW(ImGui_ImplGtkmm_Data)();
    io.BackendPlatformUserData       = (void *)bd;
    io.BackendPlatformName           = "imgui_impl_gtkmm";

    bd->window                       = window;
    bd->glArea                       = glArea;

    io.SetClipboardTextFn            = ImGui_ImplGtkmm_SetClipboardText;
    io.GetClipboardTextFn            = ImGui_ImplGtkmm_GetClipboardText;
    io.ClipboardUserData             = nullptr;
    io.SetPlatformImeDataFn          = ImGui_ImplGtkmm_SetPlatformImeData;

    // Set platform dependent data in viewport
    ImGuiViewport *main_viewport     = ImGui::GetMainViewport();
    main_viewport->PlatformHandleRaw = nullptr;

    return true;
}

void ImGui_ImplGtkmm_Shutdown() {
    ImGui_ImplGtkmm_Data *bd = ImGui_ImplGtkmm_GetBackendData();
    IM_DELETE(bd);
}

void ImGui_ImplGtkmm_NewFrame() {
    ImGui_ImplGtkmm_Data *bd = ImGui_ImplGtkmm_GetBackendData();
    IM_ASSERT(bd != nullptr && "BackendData not initialized");
    ImGuiIO &io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int      x, y, w, h;
    int      display_w, display_h;
    bd->glArea->get_bounds(x, y, w, h);
    // if (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_MINIMIZED)
    //     w = h = 0;
    // SDL_GetWindowSizeInPixels(bd->Window, &display_w, &display_h);
    io.DisplaySize                                 = ImVec2((float)w, (float)h);
    // if (w > 0 && h > 0)
    //     io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    auto                                     now   = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> delta = now - bd->last_frame_time;
    io.DeltaTime                                   = delta.count();
    bd->last_frame_time                            = now;
}

bool ImGui_ImplGtkmm_ProcessEvent(const Gdk::Event *event) {
    ImGui_ImplGtkmm_Data *bd = ImGui_ImplGtkmm_GetBackendData();
    ImGuiIO              &io = ImGui::GetIO();

    switch(event->get_event_type()) {
    case Gdk::Event::Type::KEY_PRESS:
    case Gdk::Event::Type::KEY_RELEASE: {
        bool pressed = event->get_event_type() == Gdk::Event::Type::BUTTON_PRESS;
        u32  keyval  = event->get_keyval();
        auto state   = event->get_modifier_state();

        if(!io.WantCaptureKeyboard) {
            return false;
        }

        ImGuiKey key;
#define case_set_key(_GDK_KEY, _ImGuiKey) \
    case _GDK_KEY: key = _ImGuiKey; break
        switch(keyval) {
            case_set_key(GDK_KEY_Tab, ImGuiKey_Tab);
            case_set_key(GDK_KEY_Left, ImGuiKey_LeftArrow);
            case_set_key(GDK_KEY_Right, ImGuiKey_RightArrow);
            case_set_key(GDK_KEY_Up, ImGuiKey_UpArrow);
            case_set_key(GDK_KEY_Down, ImGuiKey_DownArrow);
            case_set_key(GDK_KEY_Page_Up, ImGuiKey_PageUp);
            case_set_key(GDK_KEY_Page_Down, ImGuiKey_PageDown);
            case_set_key(GDK_KEY_Home, ImGuiKey_Home);
            case_set_key(GDK_KEY_End, ImGuiKey_End);
            case_set_key(GDK_KEY_Insert, ImGuiKey_Insert);
            case_set_key(GDK_KEY_Delete, ImGuiKey_Delete);
            case_set_key(GDK_KEY_BackSpace, ImGuiKey_Backspace);
            case_set_key(GDK_KEY_space, ImGuiKey_Space);
            case_set_key(GDK_KEY_Return, ImGuiKey_Enter);
            case_set_key(GDK_KEY_Escape, ImGuiKey_Escape);

            case_set_key(GDK_KEY_0, ImGuiKey_0);
            case_set_key(GDK_KEY_1, ImGuiKey_1);
            case_set_key(GDK_KEY_2, ImGuiKey_2);
            case_set_key(GDK_KEY_3, ImGuiKey_3);
            case_set_key(GDK_KEY_4, ImGuiKey_4);
            case_set_key(GDK_KEY_5, ImGuiKey_5);
            case_set_key(GDK_KEY_6, ImGuiKey_6);
            case_set_key(GDK_KEY_7, ImGuiKey_7);
            case_set_key(GDK_KEY_8, ImGuiKey_8);
            case_set_key(GDK_KEY_9, ImGuiKey_9);

            case_set_key(GDK_KEY_A, ImGuiKey_A);
            case_set_key(GDK_KEY_B, ImGuiKey_B);
            case_set_key(GDK_KEY_C, ImGuiKey_C);
            case_set_key(GDK_KEY_D, ImGuiKey_D);
            case_set_key(GDK_KEY_E, ImGuiKey_E);
            case_set_key(GDK_KEY_F, ImGuiKey_F);
            case_set_key(GDK_KEY_G, ImGuiKey_G);
            case_set_key(GDK_KEY_H, ImGuiKey_H);
            case_set_key(GDK_KEY_I, ImGuiKey_I);
            case_set_key(GDK_KEY_J, ImGuiKey_J);
            case_set_key(GDK_KEY_K, ImGuiKey_K);
            case_set_key(GDK_KEY_L, ImGuiKey_L);
            case_set_key(GDK_KEY_M, ImGuiKey_M);
            case_set_key(GDK_KEY_N, ImGuiKey_N);
            case_set_key(GDK_KEY_O, ImGuiKey_O);
            case_set_key(GDK_KEY_P, ImGuiKey_P);
            case_set_key(GDK_KEY_Q, ImGuiKey_Q);
            case_set_key(GDK_KEY_R, ImGuiKey_R);
            case_set_key(GDK_KEY_S, ImGuiKey_S);
            case_set_key(GDK_KEY_T, ImGuiKey_T);
            case_set_key(GDK_KEY_U, ImGuiKey_U);
            case_set_key(GDK_KEY_V, ImGuiKey_V);
            case_set_key(GDK_KEY_W, ImGuiKey_W);
            case_set_key(GDK_KEY_X, ImGuiKey_X);
            case_set_key(GDK_KEY_Y, ImGuiKey_Y);
            case_set_key(GDK_KEY_Z, ImGuiKey_Z);

            case_set_key(GDK_KEY_F1, ImGuiKey_F1);
            case_set_key(GDK_KEY_F2, ImGuiKey_F2);
            case_set_key(GDK_KEY_F3, ImGuiKey_F3);
            case_set_key(GDK_KEY_F4, ImGuiKey_F4);
            case_set_key(GDK_KEY_F5, ImGuiKey_F5);
            case_set_key(GDK_KEY_F6, ImGuiKey_F6);
            case_set_key(GDK_KEY_F7, ImGuiKey_F7);
            case_set_key(GDK_KEY_F8, ImGuiKey_F8);
            case_set_key(GDK_KEY_F9, ImGuiKey_F9);
            case_set_key(GDK_KEY_F10, ImGuiKey_F10);
            case_set_key(GDK_KEY_F11, ImGuiKey_F11);
            case_set_key(GDK_KEY_F12, ImGuiKey_F12);

            case_set_key(GDK_KEY_apostrophe, ImGuiKey_Apostrophe);
            case_set_key(GDK_KEY_comma, ImGuiKey_Comma);
            case_set_key(GDK_KEY_minus, ImGuiKey_Minus);
            case_set_key(GDK_KEY_period, ImGuiKey_Period);
            case_set_key(GDK_KEY_slash, ImGuiKey_Slash);
            case_set_key(GDK_KEY_semicolon, ImGuiKey_Semicolon);
            case_set_key(GDK_KEY_equal, ImGuiKey_Equal);
            case_set_key(GDK_KEY_braceleft, ImGuiKey_LeftBracket);
            case_set_key(GDK_KEY_backslash, ImGuiKey_Backslash);
            case_set_key(GDK_KEY_braceright, ImGuiKey_RightBracket);
            case_set_key(GDK_KEY_grave, ImGuiKey_GraveAccent);

            case_set_key(GDK_KEY_KP_0, ImGuiKey_Keypad0);
            case_set_key(GDK_KEY_KP_1, ImGuiKey_Keypad1);
            case_set_key(GDK_KEY_KP_2, ImGuiKey_Keypad2);
            case_set_key(GDK_KEY_KP_3, ImGuiKey_Keypad3);
            case_set_key(GDK_KEY_KP_4, ImGuiKey_Keypad4);
            case_set_key(GDK_KEY_KP_5, ImGuiKey_Keypad5);
            case_set_key(GDK_KEY_KP_6, ImGuiKey_Keypad6);
            case_set_key(GDK_KEY_KP_7, ImGuiKey_Keypad7);
            case_set_key(GDK_KEY_KP_8, ImGuiKey_Keypad8);
            case_set_key(GDK_KEY_KP_9, ImGuiKey_Keypad9);
            case_set_key(GDK_KEY_KP_Decimal, ImGuiKey_KeypadDecimal);
            case_set_key(GDK_KEY_KP_Divide, ImGuiKey_KeypadDivide);
            case_set_key(GDK_KEY_KP_Multiply, ImGuiKey_KeypadMultiply);
            case_set_key(GDK_KEY_KP_Subtract, ImGuiKey_KeypadSubtract);
            case_set_key(GDK_KEY_KP_Add, ImGuiKey_KeypadAdd);
            case_set_key(GDK_KEY_KP_Enter, ImGuiKey_KeypadEnter);
            case_set_key(GDK_KEY_KP_Equal, ImGuiKey_KeypadEqual);
        }
#undef case_set_key

        // TODO: convert these to key events
        io.KeyCtrl  = static_cast<bool>(state | Gdk::ModifierType::CONTROL_MASK);
        io.KeyShift = static_cast<bool>(state | Gdk::ModifierType::SHIFT_MASK);
        io.KeyAlt   = static_cast<bool>(state | Gdk::ModifierType::ALT_MASK);
        io.KeySuper = static_cast<bool>(state | Gdk::ModifierType::SUPER_MASK);

        io.AddKeyEvent(key, pressed);
        return true;
    }

    case Gdk::Event::Type::FOCUS_CHANGE: {
        io.AddFocusEvent(event->get_focus_in());
        return true;
    }

    case Gdk::Event::Type::MOTION_NOTIFY: {
        double x, y;
        event->get_position(x, y);

        double transX, transY;
        // TODO: this is deprecated in a future version of gtkmm
        bool   translated = bd->window->translate_coordinates(*bd->glArea, x, y, transX, transY);
        if(!translated && transX >= 0 && transY >= 0) {
            return false;
        }

        io.AddMousePosEvent(transX, transY);
        return true;
    }

    case Gdk::Event::Type::SCROLL: {
        double x, y;
        switch(event->get_direction()) {
        case Gdk::ScrollDirection::UP:
            x = 0.0;
            y = -1.0;
            break;
        case Gdk::ScrollDirection::DOWN:
            x = 0.0;
            y = 1.0;
            break;
        case Gdk::ScrollDirection::LEFT:
            x = 1.0;
            y = 0.0;
            break;
        case Gdk::ScrollDirection::RIGHT:
            x = -1.0;
            y = 0.0;
            break;
        case Gdk::ScrollDirection::SMOOTH:
            event->get_deltas(x, y);
            x *= -1;
            y *= -1;
            break;
        }
        io.AddMouseWheelEvent(x, y);

        return true;
    }

    case Gdk::Event::Type::BUTTON_PRESS:
    case Gdk::Event::Type::BUTTON_RELEASE: {
        if(!io.WantCaptureMouse) {
            return false;
        }

        bool pressed = event->get_event_type() == Gdk::Event::Type::BUTTON_PRESS;
        int  button  = event->get_button();

        switch(button) {
        case 1:  io.AddMouseButtonEvent(ImGuiMouseButton_Left, pressed); break;
        case 2:  io.AddMouseButtonEvent(ImGuiMouseButton_Middle, pressed); break;
        case 3:  io.AddMouseButtonEvent(ImGuiMouseButton_Right, pressed); break;
        default: break;
        }

        return false;
    }

        // case Gdk::Event::Type::PAD_RING:
        // case Gdk::Event::Type::PAD_STRIP: {
        //   auto deviceTool = event->get_device_tool();

        //   deviceTool.use_count()
        //   auto axis = event->get_axis(Gdk::AxisUse::, val)
        //   io.AddKeyAnalogEvent(ImGuiKey key, bool down, float v)
        // }

    default: return false;
    }
}
