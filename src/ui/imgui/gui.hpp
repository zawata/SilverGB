#pragma once

#include "GL/gl3w.h"
#include <SDL2/SDL.h>

#include "imgui/imgui.h"
#include "imgui/ImGuiFileBrowser.h"

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
    static GUI *createGUI(Configuration *config);

    void open_file(std::string file);

    ~GUI();

    loop_return_code_t mainLoop();
private:
    struct {
        bool loop_finish      = false;
        bool open_rom         = false;
        bool open_bios        = false;
        bool open_ctxt_menu   = false;
        bool game_loaded      = false;
        bool game_running     = false;
        bool debug_mode       = false;
    } state_flags;

    GUI(SDL_Window *w, SDL_GLContext g, Configuration *config);

    void MainOptionsWindow();

    void buildCPURegisterWindow();
    void buildIORegisterWindow();
    void buildDisassemblyWindow();
    void buildBreakpointWindow();
    //void buildMemoryViewWindow();

    SDL_Window* window;
    SDL_GLContext gl_context;

    File *rom_file;
    File *bios_file;
    GB_Core *core;
    Configuration *config;

    GLuint screen_texture;
    GLuint screen_texture_fbo;

    // file dialog handlers.
    imgui_addons::ImGuiFileBrowser rom_dialog;
    imgui_addons::ImGuiFileBrowser bios_dialog;
};