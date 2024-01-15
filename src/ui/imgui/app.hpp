#pragma once

#include <functional>

#include <argparse/argparse.hpp>

#include "gui.hpp"
#include "gb_core/core.hpp"
#include "menu.hpp"

#include "audio/audio.hpp"
#include "ois/OIS.h"

namespace Silver {
  // TODO: rename or namespace Silver::Core
  using GBCore = Silver::Core;

  struct Application {
    std::shared_ptr<Config> config;
    std::shared_ptr<GBCore> core;
    std::shared_ptr<AudioManager> audio;
    std::shared_ptr<OIS::InputManager> input;

    ImTextureID screen_texture_id;

    struct {
      bool game_running = false;
      bool debug_mode = false;
      bool show_fps = false;
      bool game_capture_input = true;
    } app_state;

    struct {
      std::function<void (
          const std::string &title,
          const std::string &filters,
          std::function<void (const std::string&)> cb
      )> openFileDialog;
      std::function<void (const std::string &title, const std::string &message)> openMessageBox;
    } window_cb;

    void onInit(int argc, const char **argv);
    void makeMenuBar(Silver::Menu *menubar);
    void onUpdateInputs(const Joypad::button_states_t &button_states);
    void onLoadRomFile(const std::string &filePath);
    void onLoadBootRomFile(const std::string &filePath);
    void onDraw();
    void onClose();

    Silver::File *rom_file, *bootrom_file;
  };
}