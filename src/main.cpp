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

    const char *file = NULL;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_STRING('f', "file", &file, "open file"),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argc = argparse_parse(&argparse, argc, argv);


    GUI *g = GUI::createGUI();

    if(file) g->open_file(file);

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
