#pragma once

#include "imgui.h"

namespace Gtk {
    struct GLArea;
    struct ApplicationWindow;
}

namespace Gdk {
    struct Event;
}

IMGUI_IMPL_API bool ImGui_ImplGtkmm_Init(Gtk::ApplicationWindow *, Gtk::GLArea *);
IMGUI_IMPL_API void ImGui_ImplGtkmm_Shutdown();
IMGUI_IMPL_API void ImGui_ImplGtkmm_NewFrame();
IMGUI_IMPL_API bool ImGui_ImplGtkmm_ProcessEvent(const Gdk::Event *event);
