#include <chrono>
#include <iostream>

#include <SDL2/SDL.h>
#include "SDL_events.h"
#include "SDL_keycode.h"

#include "gl/gl.hpp"
#include "gl/shaders/150-core.hpp"
#include "audio/audio.hpp"

#include "gb_core/core.hpp"
#include "gb_core/defs.hpp"
#include "util/file.hpp"

void set_inputs(Joypad::button_states_t *buttons, SDL_KeyboardEvent *event) {
    switch(event->keysym.sym) {
    case SDLK_UP:        // DPAD up
        buttons->up     = event->type == SDL_KEYDOWN;
        break;
    case SDLK_DOWN:      // DPAD down
        buttons->down   = event->type == SDL_KEYDOWN;
        break;
    case SDLK_LEFT:      // DPAD left
        buttons->left   = event->type == SDL_KEYDOWN;
        break;
    case SDLK_RIGHT:     // DPAD right
        buttons->right  = event->type == SDL_KEYDOWN;
        break;
    case SDLK_z:         // A button
        buttons->a      = event->type == SDL_KEYDOWN;
        break;
    case SDLK_x:         // B button
        buttons->b      = event->type == SDL_KEYDOWN;
        break;
    case SDLK_RETURN:    // Start button
        buttons->start  = event->type == SDL_KEYDOWN;
        break;
    case SDLK_BACKSPACE: // Select Button
        buttons->select = event->type == SDL_KEYDOWN;
        break;
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_GLContext context;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        nowide::cout << SDL_GetError() << std::endl;
        return -1;
    }

    // Set the desired OpenGL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create the window using SDL
    window = SDL_CreateWindow("SilverGB",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        GB_S_W * 2, GB_S_H * 2, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (window == NULL) {
        nowide::cout << SDL_GetError() << std::endl;
        return -1;
    }

    // Create the OpenGL context for the window using SDL
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        nowide::cout << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_GL_SetSwapInterval(0);

    GLuint screen_texture, shader_program, vert_buff_obj, vert_arr_obj;

    // Initialize GL
    ::init(SDL_GL_GetProcAddress);

    screen_texture = initialize_empty_texture(GB_S_W, GB_S_H);
    shader_program = link_shader_program(2,
        compile_vertex_shader(vert_shader_src),
        compile_fragment_shader(frag_shader_src)
    );
    initialize_vertex_arrays(&vert_arr_obj, &vert_buff_obj);

    Silver::File *bios_file = Silver::File::openFile("/home/johna/Documents/SilverGB/test_files/bootroms/cgb_boot.bin");
    Silver::File *rom_file = nullptr;
    Silver::Core *core = nullptr;
    GB_Audio *audio = nullptr;
    Joypad::button_states_t button_state;

    bool isRunning = true;
    SDL_Event event;
    auto start = std::chrono::steady_clock::now();
    int frames = 0;
    while (isRunning) {
        ++frames;
        auto now = std::chrono::steady_clock::now();
        auto diff = now - start;
        if(diff >= std::chrono::seconds(1)) {
            start = now;
            SDL_SetWindowTitle(window, ("SilverGB - " + std::to_string(frames)).c_str());
            frames = 0;
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                set_inputs(&button_state, &event.key);
                break;
            case SDL_DROPFILE:
                if(core) {
                    delete audio;
                    delete core;
                    delete rom_file;
                }

                nowide::cout << event.drop.file << std::endl;

                rom_file = Silver::File::openFile(event.drop.file);
                core = new Silver::Core(rom_file, bios_file);
                audio = GB_Audio::init_audio(core);
                audio->start_audio();
                break;
            case SDL_QUIT:
                isRunning = false;
                break;
            }
        }

        SDL_GL_MakeCurrent(window, context);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        begin_draw(w, h);

        if(core) {

            // run a frame
            core->set_input_state(button_state);
            core->tick_frame();

            // draw screen
            draw_screen_to_texture(screen_texture, GB_S_W, GB_S_H, core->getScreenBuffer());
            draw_display_quad(vert_arr_obj, screen_texture, shader_program);
        }

        // Swap the OpenGL window buffers
        SDL_GL_SwapWindow(window);
    }

    if(core) {
        //cleanup the core
        delete core;
    }

    // Release resources
    delete bios_file;
    delete audio;
    SDL_GL_DeleteContext(context);
    SDL_Quit();

    return 0;
}