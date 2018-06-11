#include "sdl.hpp"

GUI::GUI() {}

GUI::~GUI() {
    TTF_CloseFont(font);
    TTF_SetFontHinting(font, TTF_HINTING_MONO);

    SDL_DestroyWindow(reg_window->window);
    SDL_DestroyWindow(main_window->window);

    TTF_Quit();
    SDL_Quit();
}

GUI *GUI::createGUI() {
    TTF_Init();

    auto ret = new GUI;
    ret->font = TTF_OpenFont("opensans-sb.ttf", 16);
    if(!ret->font) return nullptr;

    ret->main_window = sdl_window_t::initWindow(640, 480, "Main Window", 0);
    if(!ret->main_window) return nullptr;

    ret->reg_window = sdl_window_t::initWindow(100, 100, "Registers", SDL_WINDOW_SKIP_TASKBAR);
    if(!ret->reg_window) return nullptr;

    return ret;
}

void GUI::showMainWindow() { SDL_ShowWindow(main_window->window);}
void GUI::hideMainWindow() { SDL_HideWindow(main_window->window);}
void GUI::showRegisterWindow() { SDL_ShowWindow(reg_window->window);}
void GUI::hideRegisterWindow() { SDL_HideWindow(reg_window->window);}

void GUI::build_reg_window() {
    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Blended( font, "test", SDL_Color({255,255,0}) );
    if( textSurface == NULL ) {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    SDL_Texture *text = SDL_CreateTextureFromSurface(reg_window->renderer, textSurface);
    if(!text) {
        printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
    }
    SDL_RenderClear( reg_window->renderer );
    SDL_Rect dst = {0, (100-textSurface->h)/2, textSurface->w, textSurface->h};
    SDL_RenderCopy( reg_window->renderer, text, NULL, &dst);
    SDL_RenderPresent( reg_window->renderer );
    SDL_DestroyTexture(text);
    SDL_FreeSurface( textSurface );
}

void GUI::renderText(sdl_window_t *w, u16 x, u16 y) {
    SDL_Surface* textSurface = TTF_RenderText_Blended( font, "test", SDL_Color({255,255,0}) );
    if(!textSurface == NULL ) std::cerr << TTF_GetError() << std::endl;
    else {
        SDL_Texture *text = SDL_CreateTextureFromSurface(reg_window->renderer, textSurface);
        if(!text) std::cerr << TTF_GetError() << std::endl;
        else {
            SDL_Rect dst = {0, (100-textSurface->h)/2, textSurface->w, textSurface->h};
            SDL_RenderCopy( reg_window->renderer, text, NULL, &dst);
            SDL_DestroyTexture(text);
        }
        SDL_FreeSurface( textSurface );
    }
}