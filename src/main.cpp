#include <iostream>

#include <SDL2/SDL.h>


#include "cart.hpp"
#include "cpu.hpp"
#include "file.hpp"
#include "gui/gui.h"
#include "sdl.hpp"
#include "mem.hpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main (int argc, char *argv[])
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_ShowSimpleMessageBox(0,
                    "SDL Error",
                    (std::string("unable to init SDL: ") + SDL_GetError()).c_str(),
                    nullptr);
    }

    return wxEntry(argc, argv);
}
    // GUI *g = GUI::createGUI();
    // if(!g) std::cout << "Error Instatiating GUI: " << SDL_GetError() << std::endl;
    // g->showMainWindow();
    // g->showRegisterWindow();
    // g->build_reg_window();
    // SDL_Delay(3000);
    // delete g ;

    // File_Interface *file;
    // if(!(file = File_Interface::openFile("tetris.gb"))) {
    //     std::cerr << "error opening file" << std::endl;
    //     return -1;
    // }

    // Cartridge *cart = new Cartridge(file);
    // printf("Code offset: %04x\n", cart->getCodeOffset());
    // std::cout << "Magic Number: " << (cart->checkMagicNumber() ? "passed" : "failed") << std::endl;
    // std::cout << "Game Title: " << cart->getCartTitle() << std::endl;
    // std::cout << "Cartridge Type: " << std::string(cart->getCartType()) << std::endl;
    // std::cout << "ROM Size: " << std::to_string(cart->getROMSize()) << std::endl;
    // std::cout << "RAM Size: " << std::to_string(cart->getRAMSize()) << std::endl;
    // std::cout << "Supports SGB: " << (cart->cartSupportsSGB() ? "Yes" : "No" ) << std::endl;
    // std::cout << "GBC Cart: " << (cart->isCGBCart() ? "Yes" : "No" ) << std::endl;
    // std::cout << "Header Checksum: " << (cart->checkHeaderChecksum() ? "passed" : "failed") << std::endl;

    // Memory_Map *m = new Memory_Map(cart);
    // CPU *c = new CPU(m);

    // while(true) {
    //     c->cycle();
    //     std::cin.get();
    // }
    // delete c;
    // delete m;

    // delete cart;
    // delete file;
