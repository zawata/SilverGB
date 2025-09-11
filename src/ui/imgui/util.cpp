#include "util.hpp"

#include <algorithm>
#include <utility>

#include "gb_core/defs.hpp"

#include "imgui/imgui.h"

std::pair<ImVec2, ImVec2> get_screen_area(ImVec2 &win_bounds) {
    // auto screen sizing and placement code
    ImVec2 img_bottom_left;
    ImVec2 img_top_right;

    float  scaling_factor = std::min(win_bounds.x / GB_S_W, win_bounds.y / GB_S_H);

    // calculate img size
    img_top_right.x       = scaling_factor * GB_S_W;
    img_top_right.y       = scaling_factor * GB_S_H;

    img_bottom_left.x     = (win_bounds.x - img_top_right.x) / 2;
    img_bottom_left.y     = (win_bounds.y - img_top_right.y) / 2;

    img_top_right.x += img_bottom_left.x;
    img_top_right.y += img_bottom_left.y;

    return std::make_pair(img_bottom_left, img_top_right);
}