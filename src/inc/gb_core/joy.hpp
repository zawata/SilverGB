#pragma once

#include <string>

#include "gb_core/mem.hpp"

#include "util/ints.hpp"

class Joypad {
public:
    struct button_states_t {
        bool a, b, start, select, up, down, left, right;
        button_states_t() { a = b = start = select = up = down = left = right = false; }
        operator std::string() const {
            // clang-format off
            return std::string("a: ")
                 + std::to_string(a)
                 + ", b: " + std::to_string(b)
                 + ", start: " + std::to_string(start)
                 + ", select: " + std::to_string(select)
                 + ", up: " + std::to_string(up)
                 + ", down: " + std::to_string(down)
                 + ", left: " + std::to_string(left)
                 + ", right: " + std::to_string(right);
            // clang-format on
        }
    };

    Joypad(Memory *mem);
    ~Joypad();

    void                    set_input_state(button_states_t state);
    Joypad::button_states_t get_input_state();

    u8   read();
    void write(u8 data);

private:
    bool            read_dir_keys, read_button_keys;
    button_states_t current_state;
    Memory         *mem;
};