#pragma once

#include "util/ints.hpp"

class Input_Manager {
public:
    struct button_states_t{
        bool a,b,start,select,up,down,left,right;
    };

    Input_Manager();
    ~Input_Manager();

    void set_input_state(button_states_t state);
    Input_Manager::button_states_t get_input_state();

    u8 read_inputs(u8 reg);
private:
    button_states_t current_state;
};