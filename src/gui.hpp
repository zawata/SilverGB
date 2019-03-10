#pragma once

#include <iostream>
#include <SDL2/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "GL/gl3w.h"
#include "nfd.h"

#include "core.hpp"
#include "cfg.hpp"

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
    static GUI *createGUI();

    void open_file(std::string file);

    ~GUI();

    loop_return_code_t mainLoop();
private:
    struct {
        bool done             = false;
        bool opening_file     = false;
        bool opening_bios     = false;
        bool debug_mode       = false;
        bool set_debug_mode   = false;
        bool game_loaded      = false;
        bool run_game         = false;
    } state_flags;

    typedef struct __shortcut {
        bool valid; //actually initialized

        bool ctrl_mod;
        bool alt_mod;
        bool shift_mod;
        char key;
    } shortcut_t;

    typedef enum {
        CTRL  = 1,
        ALT   = 2,
        SHIFT = 3,
    } mod_key;

    typedef struct __mod_arg {
        mod_key mod0;
        mod_key mod1;
        mod_key mod2;
    } mod_arg;

    GUI(SDL_Window *w, SDL_GLContext g, ImGuiIO &io);

    void buildUI();
    void buildRenderUI();
    void buildCPURegisterUI();
    void buildIORegisterUI();
    void buildDisassemblyUI();
    void buildMemoryViewUI();

    bool build_shortcut(SDL_KeyboardEvent *key);
    void clear_shortcut();
    bool shortcut_pressed(const mod_arg mods, char key);

    SDL_Window* window;
    SDL_GLContext gl_context;

    ImGuiIO& io;

    shortcut_t current_shortcut;

    Configuration *config;
    File_Interface *rom_file;
    GB_Core *core;

    GLuint screen_texture;
};