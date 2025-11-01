#include "app.hpp"

#include <argparse/argparse.hpp>
#include <chrono>
#include <memory>
#include <optional>

#include "util/log.hpp"

#include "gui/gui.hpp"
#include "gui/settings_window.hpp"
#include "imgui_internal.h"
#include "platform.hpp"

/**
 * Called as early as possible when the app starts
 * @param argc command-line argument array length
 * @param argv command-line argument array
 */
void Silver::Application::onInit(int argc, const char *argv[]) {
    argparse::ArgumentParser program("SilverGB");

    /**
     * Argument Declaration
     */
    program.add_argument("-f", "--file").help("open file").default_value(std::string(""));

    program.add_argument("-l", "--log-level")
            .help("Log Level")
            .choices("debug", "info", "warn", "error", "fatal")
            .default_value("warn");

    program.add_argument("-e", "--emu-bios").help("use eumlated bios").default_value(false).implicit_value(true);

    /**
     * Argument Parsing
     */
    try {
        program.parse_args(argc, argv);
    } catch(const std::runtime_error &err) {
        LogFatal("argparse") << err.what();
        LogFatal("argparse") << program;
        exit(-1);
    }

    Silver::getLogger().setLogLevel(program.get<std::string>("--log-level"));

    this->config         = std::make_shared<Config>();
    this->binding        = std::make_shared<Binding::Tracker>();
    this->gamepadManager = std::make_shared<GamepadManager>();

    // config->BIOS.set_bios_enabled(!program.get<bool>("--emu-bios"));
    auto filename        = program.get<std::string>("--file");
    this->onLoadRomFile(filename);
}

/**
 * Called to construct the menubar
 * @param menubar menubar instance to populate
 */
void Silver::Application::makeMenuBar(Silver::Menu *menubar) {
    /**
     * File
     */
    auto fileMenu = Menu();
    fileMenu.addItem<CallbackMenuItem>("Open file", [this](const CallbackMenuItem &, void *) {
        Platform::openFileDialog(
                "Open Rom",
                "Supported Roms:gb,gbc,bin;Gameboy ROM:gb;Gameboy Color ROM:gbc;bin",
                [this](const std::string &filepath) { this->onLoadRomFile(filepath); });
    });

    auto                           recentFilesMenu = Menu();
    CallbackMenuItem::CallbackType cb
            = [this](const CallbackMenuItem &item, void *) { this->onLoadRomFile(item.label); };
    for(auto const &file : this->recent_files) {
        LogDebug("Menu") << "Adding recent file menu item: " << file;
        recentFilesMenu.addItem<CallbackMenuItem>(file.c_str(), cb);
    }

    recentFilesMenu.addSeparator();
    recentFilesMenu.addItem<CallbackMenuItem>(
            "Clear Recent Files", [this](const CallbackMenuItem &item, void *) { this->recent_files.clear(); });

    fileMenu.addItem<SubMenuItem>("Recent Files", recentFilesMenu);

    fileMenu.addSeparator();
    fileMenu.addItem<CallbackMenuItem>("Exit", [this](const CallbackMenuItem &, void *) { this->onClose(); });
    menubar->addItem<SubMenuItem>("File", fileMenu);

    /**
     * Emulation
     */
    auto emulationMenu = Menu();
    emulationMenu.addItem<CallbackMenuItem>(
            "Reset",
            [this](const CallbackMenuItem &, void *) {
                // TODO: not sure this is safe
                this->core = std::make_shared<Silver::Core>(
                        this->rom_file,
                        this->bootrom_file == nullptr ? std::nullopt : std::make_optional(this->bootrom_file));
            },
            nullptr);
    emulationMenu.addItem<ToggleMenuItem>(
            "Paused",
            [this](const ToggleMenuItem &, bool new_state, void *) { this->app_state.game.running = !new_state; },
            nullptr,
            false);
    menubar->addItem<Silver::SubMenuItem>("Emulation", emulationMenu);

    /**
     * Window
     */
    auto windowMenu = Menu();
    windowMenu.addItem<ToggleMenuItem>("Show Options", &this->app_state.ui.show_options);
    windowMenu.addItem<ToggleMenuItem>("Show FPS", &this->app_state.ui.show_fps);
    menubar->addItem<Silver::SubMenuItem>("Window", windowMenu);

    /**
     * Debug
     */
    auto debugMenu = Menu();
    debugMenu.addItem<ToggleMenuItem>("Debug Mode", &this->app_state.debug.enabled);
    menubar->addItem<Silver::SubMenuItem>("Debug", debugMenu);
}

void Silver::Application::onLoadRomFile(const std::string &filePath) {
    if(filePath.empty()) {
        return;
    }

    this->recent_files.insert(filePath);
    this->config->file.recent_files.assign(this->recent_files.begin(), this->recent_files.end());
    Platform::createMenuBar();

    if(this->rom_file != nullptr) {
        this->app_state.game.running = false;
        this->core.reset();
        this->rom_file.reset();
    }

    auto file = Silver::File::openFile(filePath);
    if(file == nullptr) {
        LogError("App") << "Failed to open file: " << filePath;
        return;
    }

    this->rom_file = std::shared_ptr<Silver::File> {file};
    this->core     = std::make_shared<Silver::Core>(
            this->rom_file, this->bootrom_file == nullptr ? std::nullopt : std::make_optional(this->bootrom_file));
    this->app_state.game.running = true;
}

void Silver::Application::onLoadBootRomFile(const std::string &filePath) {
    if(filePath.empty()) {
        return;
    }

    auto file = Silver::File::openFile(filePath);
    if(file == nullptr) {
        LogError("App") << "Failed to open file: " << filePath;
        return;
    }

    this->bootrom_file = std::shared_ptr<Silver::File> {file};
}

float get_calc_fps() {
    using Clock = std::chrono::high_resolution_clock;
    static Clock::time_point last_invocation;
    static u64               rollingDeltaMicroseconds = 0;

    u64                      nsDelta                  = (Clock::now() - last_invocation).count();

    rollingDeltaMicroseconds += (nsDelta / 1000);
    rollingDeltaMicroseconds >>= 1;

    last_invocation = Clock::now();
    return 1000000.0f / (float)rollingDeltaMicroseconds;
}

void Silver::Application::onUpdate() {
    float fps = get_calc_fps();

    // run periodic updates
    gamepadManager->updateGamepads(this->binding);

    bool isOptionsComboPressed
            = binding->isButtonPressed(Binding::Button::Start) && binding->isButtonPressed(Binding::Button::Select);
    if(isOptionsComboPressed) {
        this->app_state.ui.show_options = !this->app_state.ui.show_options;
    }

    if(this->core) {
        // update inputs
        Joypad::button_states_t buttonsState {};
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
    buildScreenView(this);

    if(this->app_state.ui.show_options) {
        buildSettingsWindow(this);
    }

    if(this->app_state.ui.show_fps) {
        buildFpsWindow(fps);
    }

    if(this->app_state.debug.enabled) {
        buildDebugWindow(this);
    }
}

void                 Silver::Application::onClose() { exit(0); }

Silver::Application *Silver::getApp() {
    static Silver::Application app {};
    return &app;
}
