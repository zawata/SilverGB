#include <argparse/argparse.hpp>

#include "app.hpp"
#include "gui.hpp"
#include "gb_core/core.hpp"

void Silver::Application::onInit() {

}

void Silver::Application::promptToOpenRom() {
    // TODO:
}

void Silver::Application::makeMenuBar(NUI::Menu *menubar) {
    using namespace NUI;

    auto fileMenu = Menu();
    fileMenu.addItem<CallbackMenuItem>("Open file", [](const CallbackMenuItem &, void * data){
        static_cast<Silver::Application *>(data)->promptToOpenRom();
    }, this);
    fileMenu.addSeparator();
    fileMenu.addItem<CallbackMenuItem>("Exit", [](const CallbackMenuItem &, void * data){
        static_cast<Application *>(data)->close();
    }, this);
    menubar->addItem<SubMenuItem>("File", fileMenu);

    auto emulationMenu = Menu();
    emulationMenu.addItem<ToggleMenuItem>("Paused", [](const ToggleMenuItem &, bool new_state, void *data){
        static_cast<Application *>(data)->state_flags.game_running = !new_state;
    }, this, false);
    menubar->addItem<SubMenuItem>("Emulation", emulationMenu);
}

void Silver::Application::onLoad(int argc, char **argv) {
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

    this->config = new Config();

    // config->BIOS.set_bios_enabled(!program.get<bool>("--emu-bios"));
    std::string filename = program.get<std::string>("--file");
    GUI *g = GUI::createGUI(config);
    if(!filename.empty()) g->open_file(filename);
}

void Silver::Application::onDraw() {
    try {
        if(this->state_flags.game_running) {
            this->core->tick_frame();
        }
    }
    catch(breakpoint_exception &e) {
        state_flags.game_running = false;
    }

    g->mainLoop();
}

void Silver::Application::close() {
    //TODO
    exit(0);
}