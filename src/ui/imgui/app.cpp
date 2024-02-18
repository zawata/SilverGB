#include <chrono>
#include <argparse/argparse.hpp>
#include <memory>

#include "app.hpp"
#include "gui.hpp"
#include "imgui.h"
#include "imgui_internal.h"

void Silver::Application::onInit(int argc, const char *argv[]) {
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
    catch(const std::runtime_error &err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        exit(0);
    }

    this->config = std::make_shared<Config>();
    this->binding = std::make_shared<Binding::Tracker>();
    this->gamepadManager = std::make_shared<GamepadManager>();

    // config->BIOS.set_bios_enabled(!program.get<bool>("--emu-bios"));
    auto filename = program.get<std::string>("--file");
    this->onLoadRomFile(filename);
}

void Silver::Application::makeMenuBar(Silver::Menu *menubar) {
    /**
     * File
     */
    auto fileMenu = Menu();
    fileMenu.addItem<CallbackMenuItem>("Open file", [this](const CallbackMenuItem &, void *) {
        this->window_cb.openFileDialog(
                "Open Rom",
                "Gameboy ROM:gb,bin",
                [this](const std::string &filepath) {
                    this->onLoadRomFile(filepath);
                }
        );
    });
    fileMenu.addSeparator();
    fileMenu.addItem<CallbackMenuItem>("Exit", [this](const CallbackMenuItem &, void *) {
        // this->close();
    });
    menubar->addItem<SubMenuItem>("File", fileMenu);

    /**
     * Emulation
     */
    auto emulationMenu = Menu();
    emulationMenu.addItem<CallbackMenuItem>("Reset", [this](const CallbackMenuItem &, void *) {
        // TODO: not sure this is safe
        this->core = std::make_shared<Silver::Core>(this->rom_file, this->bootrom_file);
    }, nullptr);
    emulationMenu.addItem<ToggleMenuItem>("Paused", [this](const ToggleMenuItem &, bool new_state, void *) {
        this->app_state.game.running = !new_state;
    }, nullptr, false);
    menubar->addItem<Silver::SubMenuItem>("Emulation", emulationMenu);

    /**
     * Window
     */
    auto windowMenu = Menu();
    windowMenu.addItem<ToggleMenuItem>("Show Options", &this->app_state.ui.show_options);
    windowMenu.addItem<ToggleMenuItem>("Show FPS", &this->app_state.ui.show_fps);
    menubar->addItem<Silver::SubMenuItem>("Window", windowMenu);

}

void Silver::Application::onLoadRomFile(const std::string &filePath) {
    if(filePath.empty()) {
        return;
    }

    this->rom_file = Silver::File::openFile(filePath);
    this->core = std::make_shared<Silver::Core>(this->rom_file, this->bootrom_file);
    this->app_state.game.running = true;
}

void Silver::Application::onLoadBootRomFile(const std::string &filePath) {
    if(filePath.empty()) {
        return;
    }

    this->bootrom_file = Silver::File::openFile(filePath);
}

float get_calc_fps() {
    using Clock = std::chrono::high_resolution_clock;
    static Clock::time_point last_invocation;
    static u64 rollingDeltaMicroseconds = 0;

    u64 nsDelta = (Clock::now() - last_invocation).count();

    rollingDeltaMicroseconds += (nsDelta / 1000);
    rollingDeltaMicroseconds >>= 1;

    last_invocation = Clock::now();
    return 1000000.0f / (float) rollingDeltaMicroseconds;
}

void get_screen_area(const ImVec2 &win_bounds, ImVec2 &top_left, ImVec2 &bottom_right) {
    //auto screen sizing and placement code
    float scaling_factor =
            std::fmin(win_bounds.x / Silver::Core::native_width, win_bounds.y / Silver::Core::native_height);
    float width = scaling_factor * Silver::Core::native_width;
    float height = scaling_factor * Silver::Core::native_height;
    float wRemainder = win_bounds.x - width;
    float hRemainder = win_bounds.y - height;

    top_left = {wRemainder / 2.0f, hRemainder / 2.0f};
    bottom_right = {top_left.x + width, top_left.y + height};
}

void Silver::Application::onUpdate() {
    float fps = get_calc_fps();

    // run periodic updates
    gamepadManager->updateGamepads(this->binding);

    bool isOptionsComboPressed = binding->isButtonPressed(Binding::Button::Start)
            && binding->isButtonPressed(Binding::Button::Select);
    if(isOptionsComboPressed) {
        this->app_state.ui.show_options = !this->app_state.ui.show_options;
    }

    if(this->core) {
        // update inputs
        Joypad::button_states_t buttonsState{};
        binding->getButtonStates(buttonsState);
        this->core->set_input_state(buttonsState);

        try {
            if(this->app_state.game.running) {
                this->core->tick_frame();
            }
        } catch(breakpoint_exception &e) {
            this->app_state.game.running = false;
        }
    }

    // draw screen
    ImVec2 top_left, bottom_right;
    get_screen_area(ImGui::GetMainViewport()->WorkSize, top_left, bottom_right);
    ImGui::GetBackgroundDrawList()->AddImage((void *)this->screen_texture_id, top_left, bottom_right);

    if(this->app_state.ui.show_options) {
        buildOptionsWindow(this);
    }

    if(this->app_state.ui.show_fps) {
        buildFpsWindow(fps);
    }

    ImGui::ShowDemoWindow();
}

void Silver::Application::onClose() {
    //TODO
    exit(0);
}