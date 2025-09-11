#pragma once

#include "../app.hpp"

enum SettingsSection { Emulation, Display, Audio, Input, Debug, About };

void buildSettingsWindow(Silver::Application *app);