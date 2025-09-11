#pragma once

#include <argparse/argparse.hpp>
#include <functional>

#include "gb_core/core.hpp"

#include "audio/audio.hpp"
#include "binding.hpp"
#include "cfg.hpp"
#include "gui/gui.hpp"
#include "input/gamepad.hpp"
#include "menu.hpp"

namespace Silver {
    // TODO: rename or namespace Silver::Core
    using GBCore = Silver::Core;

    struct Application {
        std::shared_ptr<Config>           config;
        std::shared_ptr<GBCore>           core;
        std::shared_ptr<AudioManager>     audio;
        std::shared_ptr<Binding::Tracker> binding;
        std::shared_ptr<GamepadManager>   gamepadManager;

        void                             *screen_texture_id;
        void                             *debug_bg_texture_id;
        void                             *vram_debug_texture_ids[6];

        struct {
            struct {
                bool running;
            } game;
            struct {
                bool show_fps     = false;
                bool show_options = false;
            } ui;
            struct {
                bool enabled          = false;
                bool drawToBackground = true;
            } debug;
        } app_state;

        struct {
            std::function<void(
                    const std::string &title, const std::string &filters, std::function<void(const std::string &)> cb)>
                                                                                      openFileDialog;
            std::function<void(const std::string &title, const std::string &message)> openMessageBox;
        } window_cb;

        void          onInit(int argc, const char **argv);
        void          makeMenuBar(Silver::Menu *menubar);
        void          onLoadRomFile(const std::string &filePath);
        void          onLoadBootRomFile(const std::string &filePath);
        void          onUpdate();
        void          onClose();

        Silver::File *rom_file, *bootrom_file;
    };

    Application *getApp();
} // namespace Silver