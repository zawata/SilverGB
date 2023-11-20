#pragma once

#include "imgui/imgui.h"
// #include "imgui/ImGuiFileBrowser.h"

#include "gb_core/core.hpp"
#include "cfg/cfg.hpp"

#define DMG_BIOS_CRC 0x59c8598e

#define MENUBAR_HEIGHT 19

class GUI {
public:
    enum loop_return_code_t {
        LOOP_FINISH,
        LOOP_CONTINUE,
        LOOP_ERROR
    };

    static bool preInitialize();
    static GUI *createGUI(Config *config);

    void open_file(std::string file);

    ~GUI();

    loop_return_code_t mainLoop();
private:
    GUI(Config *config);

    void MainOptionsWindow();

    void buildCPURegisterWindow();
    void buildIORegisterWindow();
    void buildDisassemblyWindow();
    void buildBreakpointWindow();
    //void buildMemoryViewWindow();

    Silver::File *rom_file;
    Silver::File *bios_file;
    Silver::Core *core;
    Config *config;

    // file dialog handlers.
    // imgui_addons::ImGuiFileBrowser rom_dialog;
    // imgui_addons::ImGuiFileBrowser bios_dialog;
};