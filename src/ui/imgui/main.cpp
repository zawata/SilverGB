#include "gui.hpp"

#include "argparse.h"

static const char *const usage[] = {
    "main [options] [[--] args]",
    "main [options]",
    NULL,
};

int main(int argc, const char **argv) {
    if(!GUI::preInitialize()) {
        printf("%s\n", SDL_GetError());
        return -1;
    }

    /**
     * Argument Declaration
     */
    const char *file = NULL;
    bool emu_bios;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_STRING('f', "file", &file, "open file"),
        //OPT_BOOLEAN('r', "real-bios", &real_bios, "use real bios"),
        OPT_BOOLEAN('b', "emu-bios", &emu_bios, "use eumlated bios"),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argc = argparse_parse(&argparse, argc, argv);

    /**
     * Argument Parsing
     */
    Configuration *config = Configuration::loadConfigFile("config.cfg");
    if(!config) {
        std::cout << "creating new Config" << std::endl;
        //the config doesn't exist so lets just start a new one
        config = Configuration::newConfigFile("config.cfg");
    }

    config->BIOS.set_bios_enabled(!emu_bios);

    GUI *g = GUI::createGUI(config);
    if(file) g->open_file(file);

    int code = 0;
    while((code = g->mainLoop()) == GUI::loop_return_code_t::LOOP_CONTINUE);
    delete g;
    switch(code) {
    case GUI::loop_return_code_t::LOOP_FINISH :
        std::cout << "Exiting" << std::endl;
        return 0;
    case GUI::loop_return_code_t::LOOP_ERROR :
        std::cerr << "There was an Error..." << std::endl; //TODO: find someway to figure out what the error is
        return -1;
    default:
        std::cerr << "wat: " << code << std::endl;
        return -1;
    }
}
