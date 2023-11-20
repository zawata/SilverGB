#include <argparse/argparse.hpp>

#include "gui.hpp"
#include "gb_core/core.hpp"
#include "platform/common/menu.hpp"

namespace Silver {
    // TODO: rename or namespace Silver::Core
    using GBCore = Silver::Core;

    struct Application {
        GUI *g;
        Config *config;
        GBCore* core;

        struct {
            bool loop_finish      = false;
            bool open_rom         = false;
            bool open_bios        = false;
            bool open_ctxt_menu   = false;
            bool game_loaded      = false;
            bool game_running     = false;
            bool debug_mode       = false;
        } state_flags;

        void onInit();
        void promptToOpenRom();
        void makeMenuBar(NUI::Menu *menubar);
        void onLoad(int argc, char **argv);
        void onDraw();
        void close();
    };
}