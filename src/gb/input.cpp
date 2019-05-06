#include "gb/input.hpp"

#include "util/bit.hpp"

Input_Manager::Input_Manager() {}
Input_Manager::~Input_Manager() {}

void Input_Manager::set_input_state(button_states_t state) {
    current_state = state;
}

Input_Manager::button_states_t Input_Manager::get_input_state() {
    return current_state;
}

u8 Input_Manager::read_inputs(u8 reg) {
    reg &= 0x30;
    if(Bit.test(reg, 4)) {
        reg |=
            current_state.down  << 3 |
            current_state.up    << 2 |
            current_state.left  << 1 |
            current_state.right << 0;
    }

    if(Bit.test(reg,5 )) {
        reg |=
            current_state.start  << 3 |
            current_state.select << 2 |
            current_state.b      << 1 |
            current_state.a      << 0;
    }

    return reg;
}