#pragma once

#include <algorithm>
#include <utility>

#include "imgui/imgui.h"

std::pair<ImVec2, ImVec2> get_screen_area(ImVec2 &win_bounds);