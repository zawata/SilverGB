#include "gb_core/input.hpp"

#include "util/bit.hpp"

#include <iostream>

Input_Manager::Input_Manager() {
    read_dir_keys = false;
    read_button_keys = false;
}
Input_Manager::~Input_Manager() {}

void Input_Manager::set_input_state(button_states_t state) {
    current_state = state;
}

Input_Manager::button_states_t Input_Manager::get_input_state() {
    return current_state;
}

u8 Input_Manager::read() {
    u8 bits = 0;

    if(read_dir_keys)    Bit.set(&bits, 4);
    if(read_button_keys) Bit.set(&bits, 5);

    if(read_dir_keys) {
        bits |=
            current_state.down  << 3 |
            current_state.up    << 2 |
            current_state.left  << 1 |
            current_state.right << 0;
    }

    if(read_button_keys) {
        bits |=
            current_state.start  << 3 |
            current_state.select << 2 |
            current_state.b      << 1 |
            current_state.a      << 0;
    }

    return ~bits & 0xF;
}

void Input_Manager::write(u8 data) {
    read_dir_keys = Bit.test(data, 5);
    read_button_keys = Bit.test(data, 4);
}