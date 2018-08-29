#pragma once

#include <iostream>
#include <SDL2/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "GL/gl3w.h"
#include "nfd.h"

#include "util.hpp"

#define IMAGE_FORMAT_SIZE 3 //because RGB...for now

#define GB_S_W 160                           //screen width
#define GB_S_H 144                           //screen height
#define GB_S_P 23040                         //screen pixel count
#define GB_S_P_SZ GB_S_P * IMAGE_FORMAT_SIZE //screen pixel buffer size

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

    ~GUI();

    loop_return_code_t mainLoop();
private:
    struct {
        bool done             = false;
        bool opening_file     = false;
        bool debug_mode       = false;
        bool set_debug_mode   = false;
    } state_flags;

    GUI(SDL_Window *w, SDL_GLContext g, ImGuiIO &io);

    void buildUI();
    void buildRenderUI();
    void buildRegisterUI();
    void buildDisassemblyUI();

    SDL_Window* window;
    SDL_GLContext gl_context;

    ImGuiIO& io;

    GLuint screen_texture;
    u8 *screen_buffer;
};

#endif