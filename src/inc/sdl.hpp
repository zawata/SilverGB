#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "util.hpp"

#ifndef GUI_HPP
#define GUI_HPP

class GUI {
public:
    ~GUI();

    static GUI *createGUI();

    void build_reg_window();

    void showMainWindow();
    void hideMainWindow();

    void showRegisterWindow();
    void hideRegisterWindow();
private:
    typedef struct __sdl_window_t {
        static struct __sdl_window_t *initWindow(int w, int h, std::string title, int flags) {
            SDL_Window *window;
            //SDL_Surface *surface;
            SDL_Renderer *renderer;

            SDL_CreateWindowAndRenderer(
                    w,
                    h,
                    SDL_WINDOW_HIDDEN | flags,
                    &window,
                    &renderer);
            if(!window || !renderer) return nullptr;

            // surface = SDL_GetWindowSurface(window);
            // if(!surface) return nullptr;

            SDL_SetWindowTitle(window, title.c_str());
            SDL_SetRenderDrawColor(renderer, 0,0,0,0xFF);
            SDL_SetWindowResizable(window, SDL_FALSE);

            //return new __sdl_window_t({ window, surface, renderer });
            return new __sdl_window_t({ window, renderer });
        }

        ~__sdl_window_t() {
            //SDL_FreeSurface(surface);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
        }

        SDL_Window *window;
        //SDL_Surface *surface;
        SDL_Renderer *renderer;
    } sdl_window_t;

    GUI();

    void renderText(sdl_window_t *w, u16 x, u16 y);

    TTF_Font *font;

    sdl_window_t *main_window;
    sdl_window_t *reg_window;
};

#endif