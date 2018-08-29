#pragma once

#include "util.hpp"

class Input_Manager {
public:
    Input_Manager();
    ~Input_Manager();

    u8 read_inputs(u8 reg);
};