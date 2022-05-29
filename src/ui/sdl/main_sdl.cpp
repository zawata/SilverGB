#include <SDL_render.h>
#include <SDL_video.h>
#include <chrono>

// remove win32 mix/max defines
#define NOMINMAX
#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_keycode.h>

#include "gb_core/core.hpp"
#include "gb_core/defs.hpp"

#include "audio.hpp"

bool enable_bg_window = false;
bool enable_wnd_window = false;

float default_zoom = 3.0f;

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

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        nowide::cout << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window_screen = SDL_CreateWindow("SilverGB", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160 * default_zoom, 144 * default_zoom, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer_screen = SDL_CreateRenderer(window_screen, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    SDL_Texture *screen_texture = SDL_CreateTexture(renderer_screen, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    SDL_Window   *window_bg;
    SDL_Renderer *renderer_bg;
    SDL_Texture  *bg_texture;

    SDL_Window   *window_wnd;
    SDL_Renderer *renderer_wnd;
    SDL_Texture  *wnd_texture;

    if(enable_bg_window) {
        window_bg    = SDL_CreateWindow("SilverGB Background", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 256, SDL_WINDOW_RESIZABLE);
        renderer_bg  = SDL_CreateRenderer(window_bg, -1, SDL_RENDERER_ACCELERATED);
        bg_texture   = SDL_CreateTexture(renderer_bg, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    }

    if(enable_wnd_window) {
        window_wnd   = SDL_CreateWindow("SilverGB Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 256, SDL_WINDOW_RESIZABLE);
        renderer_wnd = SDL_CreateRenderer(window_wnd, -1, SDL_RENDERER_ACCELERATED);
        wnd_texture  = SDL_CreateTexture(renderer_wnd, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    }

    Silver::File *bios_file = Silver::File::openFile("/home/johna/Documents/SilverGB/test_files/bootroms/cgb_boot.bin");
    Silver::File *rom_file = nullptr;
    Silver::Core *core = nullptr;
    GB_Audio *audio = nullptr;
    Joypad::button_states_t button_state;

    u8 *buf = (u8 *)malloc(256 * 256 * 3);

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
            SDL_SetWindowTitle(window_screen, ("SilverGB - " + std::to_string(frames)).c_str());
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
                // audio->start_audio();
                break;
            case SDL_QUIT:
                isRunning = false;
                break;
            }
        }

        if(core) {
            core->set_input_state(button_state);
            core->tick_frame();

            void *screen_dest;
            int screen_pitch;
            SDL_LockTexture(screen_texture, NULL, &screen_dest, &screen_pitch);
            memcpy(screen_dest, core->getScreenBuffer(), GB_S_P_SZ);
            SDL_UnlockTexture(screen_texture);
            SDL_RenderCopy(renderer_screen, screen_texture, NULL, NULL);
        }

        SDL_RenderPresent(renderer_screen);

        if(enable_bg_window) {
            if(core) {
                void *bg_dest;
                int bg_pitch;
                SDL_LockTexture(bg_texture, NULL, &bg_dest, &bg_pitch);
                core->getBGBuffer(buf);
                memcpy(bg_dest, buf, 256 * 256 * 3);
                SDL_UnlockTexture(bg_texture);
                SDL_RenderCopy(renderer_bg, bg_texture, NULL, NULL);
            }
            SDL_RenderPresent(renderer_bg);
        }

        if(enable_wnd_window) {
            if(core) {
                void *wnd_dest;
                int wnd_pitch;
                SDL_LockTexture(wnd_texture, NULL, &wnd_dest, &wnd_pitch);
                core->getWNDBuffer(buf);
                memcpy(wnd_dest, buf, 256 * 256 * 3);
                SDL_UnlockTexture(wnd_texture);
                SDL_RenderCopy(renderer_wnd, wnd_texture, NULL, NULL);
            }
            SDL_RenderPresent(renderer_wnd);
        }
    }

    if(core) {
        //cleanup the core
        delete core;
    }

    delete rom_file;
    delete bios_file;
    delete audio;

    if(enable_wnd_window) {
        SDL_DestroyTexture(wnd_texture);
        SDL_DestroyRenderer(renderer_wnd);
        SDL_DestroyWindow(window_wnd);
    }

    if(enable_bg_window) {
        SDL_DestroyTexture(bg_texture);
        SDL_DestroyRenderer(renderer_bg);
        SDL_DestroyWindow(window_bg);
    }
    SDL_DestroyTexture(screen_texture);
    SDL_DestroyRenderer(renderer_screen);
    SDL_DestroyWindow(window_screen);

    free(buf);
    SDL_Quit();

    return 0;
}