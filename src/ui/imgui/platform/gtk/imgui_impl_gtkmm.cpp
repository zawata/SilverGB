#include "imgui_impl_gtkmm.hpp"

#include <chrono>
#include <gdkmm/enums.h>
#include <gdkmm/event.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/glarea.h>
#include <iostream>
#include <mutex>

#include "util/util.hpp"

#include "imgui.h"

struct ImGui_ImplGtkmm_Data {
    Gtk::Window                                   *window;
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

bool ImGui_ImplGtkmm_Init(Gtk::Window *window, Gtk::GLArea *glArea) {
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
        switch(keyval) {
        case GDK_KEY_Tab:         key = ImGuiKey_Tab;
        case GDK_KEY_Left:        key = ImGuiKey_LeftArrow;
        case GDK_KEY_Right:       key = ImGuiKey_RightArrow;
        case GDK_KEY_Up:          key = ImGuiKey_UpArrow;
        case GDK_KEY_Down:        key = ImGuiKey_DownArrow;
        case GDK_KEY_Page_Up:     key = ImGuiKey_PageUp;
        case GDK_KEY_Page_Down:   key = ImGuiKey_PageDown;
        case GDK_KEY_Home:        key = ImGuiKey_Home;
        case GDK_KEY_End:         key = ImGuiKey_End;
        case GDK_KEY_Insert:      key = ImGuiKey_Insert;
        case GDK_KEY_Delete:      key = ImGuiKey_Delete;
        case GDK_KEY_BackSpace:   key = ImGuiKey_Backspace;
        case GDK_KEY_space:       key = ImGuiKey_Space;
        case GDK_KEY_Return:      key = ImGuiKey_Enter;
        case GDK_KEY_Escape:      key = ImGuiKey_Escape;

        case GDK_KEY_0:           key = ImGuiKey_0;
        case GDK_KEY_1:           key = ImGuiKey_1;
        case GDK_KEY_2:           key = ImGuiKey_2;
        case GDK_KEY_3:           key = ImGuiKey_3;
        case GDK_KEY_4:           key = ImGuiKey_4;
        case GDK_KEY_5:           key = ImGuiKey_5;
        case GDK_KEY_6:           key = ImGuiKey_6;
        case GDK_KEY_7:           key = ImGuiKey_7;
        case GDK_KEY_8:           key = ImGuiKey_8;
        case GDK_KEY_9:           key = ImGuiKey_9;

        case GDK_KEY_A:           key = ImGuiKey_A;
        case GDK_KEY_B:           key = ImGuiKey_B;
        case GDK_KEY_C:           key = ImGuiKey_C;
        case GDK_KEY_D:           key = ImGuiKey_D;
        case GDK_KEY_E:           key = ImGuiKey_E;
        case GDK_KEY_F:           key = ImGuiKey_F;
        case GDK_KEY_G:           key = ImGuiKey_G;
        case GDK_KEY_H:           key = ImGuiKey_H;
        case GDK_KEY_I:           key = ImGuiKey_I;
        case GDK_KEY_J:           key = ImGuiKey_J;
        case GDK_KEY_K:           key = ImGuiKey_K;
        case GDK_KEY_L:           key = ImGuiKey_L;
        case GDK_KEY_M:           key = ImGuiKey_M;
        case GDK_KEY_N:           key = ImGuiKey_N;
        case GDK_KEY_O:           key = ImGuiKey_O;
        case GDK_KEY_P:           key = ImGuiKey_P;
        case GDK_KEY_Q:           key = ImGuiKey_Q;
        case GDK_KEY_R:           key = ImGuiKey_R;
        case GDK_KEY_S:           key = ImGuiKey_S;
        case GDK_KEY_T:           key = ImGuiKey_T;
        case GDK_KEY_U:           key = ImGuiKey_U;
        case GDK_KEY_V:           key = ImGuiKey_V;
        case GDK_KEY_W:           key = ImGuiKey_W;
        case GDK_KEY_X:           key = ImGuiKey_X;
        case GDK_KEY_Y:           key = ImGuiKey_Y;
        case GDK_KEY_Z:           key = ImGuiKey_Z;

        case GDK_KEY_F1:          key = ImGuiKey_F1;
        case GDK_KEY_F2:          key = ImGuiKey_F2;
        case GDK_KEY_F3:          key = ImGuiKey_F3;
        case GDK_KEY_F4:          key = ImGuiKey_F4;
        case GDK_KEY_F5:          key = ImGuiKey_F5;
        case GDK_KEY_F6:          key = ImGuiKey_F6;
        case GDK_KEY_F7:          key = ImGuiKey_F7;
        case GDK_KEY_F8:          key = ImGuiKey_F8;
        case GDK_KEY_F9:          key = ImGuiKey_F9;
        case GDK_KEY_F10:         key = ImGuiKey_F10;
        case GDK_KEY_F11:         key = ImGuiKey_F11;
        case GDK_KEY_F12:         key = ImGuiKey_F12;

        case GDK_KEY_apostrophe:  key = ImGuiKey_Apostrophe;
        case GDK_KEY_comma:       key = ImGuiKey_Comma;
        case GDK_KEY_minus:       key = ImGuiKey_Minus;
        case GDK_KEY_period:      key = ImGuiKey_Period;
        case GDK_KEY_slash:       key = ImGuiKey_Slash;
        case GDK_KEY_semicolon:   key = ImGuiKey_Semicolon;
        case GDK_KEY_equal:       key = ImGuiKey_Equal;
        case GDK_KEY_braceleft:   key = ImGuiKey_LeftBracket;
        case GDK_KEY_backslash:   key = ImGuiKey_Backslash;
        case GDK_KEY_braceright:  key = ImGuiKey_RightBracket;
        case GDK_KEY_grave:       key = ImGuiKey_GraveAccent;

        case GDK_KEY_KP_0:        key = ImGuiKey_Keypad0;
        case GDK_KEY_KP_1:        key = ImGuiKey_Keypad1;
        case GDK_KEY_KP_2:        key = ImGuiKey_Keypad2;
        case GDK_KEY_KP_3:        key = ImGuiKey_Keypad3;
        case GDK_KEY_KP_4:        key = ImGuiKey_Keypad4;
        case GDK_KEY_KP_5:        key = ImGuiKey_Keypad5;
        case GDK_KEY_KP_6:        key = ImGuiKey_Keypad6;
        case GDK_KEY_KP_7:        key = ImGuiKey_Keypad7;
        case GDK_KEY_KP_8:        key = ImGuiKey_Keypad8;
        case GDK_KEY_KP_9:        key = ImGuiKey_Keypad9;
        case GDK_KEY_KP_Decimal:  key = ImGuiKey_KeypadDecimal;
        case GDK_KEY_KP_Divide:   key = ImGuiKey_KeypadDivide;
        case GDK_KEY_KP_Multiply: key = ImGuiKey_KeypadMultiply;
        case GDK_KEY_KP_Subtract: key = ImGuiKey_KeypadSubtract;
        case GDK_KEY_KP_Add:      key = ImGuiKey_KeypadAdd;
        case GDK_KEY_KP_Enter:    key = ImGuiKey_KeypadEnter;
        case GDK_KEY_KP_Equal:    key = ImGuiKey_KeypadEqual;
        default:                  key = ImGuiKey_None;
        }

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
        auto   maybe_point = bd->window->compute_point(*bd->glArea, Gdk::Graphene::Point(x, y));

        if(!maybe_point.has_value()) {
            return false;
        }

        transX = maybe_point.value().get_x();
        transY = maybe_point.value().get_y();

        io.AddMousePosEvent((float)transX, (float)transY);
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
        io.AddMouseWheelEvent((float)x, (float)y);

        return true;
    }

    case Gdk::Event::Type::BUTTON_PRESS:
    case Gdk::Event::Type::BUTTON_RELEASE: {
        if(!io.WantCaptureMouse) {
            return false;
        }

        bool pressed = event->get_event_type() == Gdk::Event::Type::BUTTON_PRESS;
        uint button  = event->get_button();

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
