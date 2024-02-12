#pragma once

#include "app.hpp"
#include "binding.hpp"
#include "imgui/imgui.h"
// #include "imgui/ImGuiFileBrowser.h"

#include "gb_core/core.hpp"
#include "cfg.hpp"
#include <memory>

namespace Silver {
    struct Application;
};

#define DMG_BIOS_CRC 0x59c8598e

void buildFpsWindow(float fps);
void buildOptionsWindow(Silver::Application *app);
void buildCPURegisterWindow(Silver::Core* core);
void buildIORegisterWindow(Silver::Core* core);
void buildBreakpointWindow(Silver::Core *core);

class GUI {
public:
    enum loop_return_code_t {
        LOOP_FINISH,
        LOOP_CONTINUE,
        LOOP_ERROR
    };

    GUI(Config *config);
    ~GUI();
private:
    void buildDisassemblyWindow();
    //void buildMemoryViewWindow();

    Config *config;
};