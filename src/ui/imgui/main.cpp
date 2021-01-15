#include "gui.hpp"

#include "argparse/argparse.hpp"

int main(int argc, char **argv) {
    if(!GUI::preInitialize()) {
        printf("%s\n", SDL_GetError());
        return -1;
    }

    argparse::ArgumentParser program("SilverGB");

    /**
     * Argument Declaration
     */
    program.add_argument("-f", "--file")
        .help("open file")
        .default_value(std::string(""));

    program.add_argument("-e", "--emu-bios")
        .help("use eumlated bios")
        .default_value(false)
        .implicit_value(true);

    /**
     * Argument Parsing
     */
    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(0);
    }


    Configuration *config = Configuration::loadConfigFile("config.cfg");
    if(!config) {
        std::cout << "creating new Config" << std::endl;
        //the config doesn't exist so lets just start a new one
        config = Configuration::newConfigFile("config.cfg");
        assert(config);
    }

    config->BIOS.set_bios_enabled(!program.get<bool>("--emu-bios"));

    std::string filename = program.get<std::string>("--file");
    GUI *g = GUI::createGUI(config);
    if(!filename.empty()) g->open_file(filename);

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
