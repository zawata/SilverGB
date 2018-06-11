#include <iostream>
#include <SDL2/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "GL/gl3w.h"
#include "nfd.h"

#include "util.hpp"

#ifndef GUI_HPP
#define GUI_HPP

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
        bool done         = false;
        bool opening_file = false;
    } state_flags;

    GUI(SDL_Window *w, SDL_GLContext g, ImGuiIO &io);

    void buildUI();
    void buildRegisterUI();

    SDL_Window* window;
    SDL_GLContext gl_context;

    ImGuiIO& io;
};

#endif