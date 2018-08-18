#include "gui.hpp"

int main(int, char**)
{
    if(!GUI::preInitialize()) {
        printf("%s\n", SDL_GetError());
        return -1;
    }

    GUI *g = GUI::createGUI();
    int code;
    while((code = g->mainLoop()) == GUI::loop_return_code_t::LOOP_CONTINUE);
    delete g;
    if(code == GUI::loop_return_code_t::LOOP_FINISH) {
        std::cout << "Exiting" << std::endl;
        return 0;
    } else if (code == GUI::loop_return_code_t::LOOP_ERROR) {
        std::cerr << "There was an Error. TODO: find someway to figure out what the error is" << std::endl;
        return -1;
    } else {
        std::cerr << "wat: " << code << std::endl;
        return -1;
    }

}
