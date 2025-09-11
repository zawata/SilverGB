#pragma once

#include "gb_core/core.hpp"

#include "app.hpp"
#include "cfg.hpp"

namespace Silver {
    struct Application;
}; // namespace Silver

#define DMG_BIOS_CRC 0x59c8598e

void buildScreenView(Silver::Application *app);
void buildFpsWindow(float fps);
void buildDebugWindow(Silver::Application *app);
void buildCPURegisterWindow(Silver::Core *core);
void buildIORegisterWindow(Silver::Core *core);
void buildBreakpointWindow(Silver::Core *core);

class GUI {
public:
    enum loop_return_code_t { LOOP_FINISH, LOOP_CONTINUE, LOOP_ERROR };

    GUI(Config *config);
    ~GUI();

private:
    void    buildDisassemblyWindow();
    // void buildMemoryViewWindow();

    Config *config;
};