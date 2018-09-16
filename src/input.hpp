#pragma once

#include "util/ints.hpp"

class Input_Manager {
public:
    Input_Manager();
    ~Input_Manager();

    u8 read_inputs(u8 reg);
};