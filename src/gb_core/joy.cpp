#include "gb_core/joy.hpp"

#include <nowide/iostream.hpp>

#include "util/bit.hpp"

Joypad::Joypad(Memory *mem): mem(mem) {
    read_dir_keys    = false;
    read_button_keys = false;
}
Joypad::~Joypad() {}

void Joypad::set_input_state(button_states_t state) {
    current_state = state;

    if(read_dir_keys) {
        if(current_state.down || current_state.up || current_state.left || current_state.right) {
            mem->request_interrupt(Memory::Interrupt::JOYPAD_INT);
        }
    }

    if(read_button_keys) {
        if(current_state.start || current_state.select || current_state.b || current_state.a) {
            mem->request_interrupt(Memory::Interrupt::JOYPAD_INT);
        }
    }
}

Joypad::button_states_t Joypad::get_input_state() {
    return current_state;
}

u8 Joypad::read() {
    u8 bits = 0;

    if(read_dir_keys)
        Bit::set(&bits, 4);
    if(read_button_keys)
        Bit::set(&bits, 5);

    if(read_dir_keys) {
        bits |= current_state.down << 3 | current_state.up << 2 | current_state.left << 1 | current_state.right << 0;
    }

    if(read_button_keys) {
        bits |= current_state.start << 3 | current_state.select << 2 | current_state.b << 1 | current_state.a << 0;
    }

    return ~bits & 0xF;
}

void Joypad::write(u8 data) {
    read_dir_keys    = Bit::test(data, 5);
    read_button_keys = Bit::test(data, 4);
}