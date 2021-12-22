#pragma once

#include <nowide/iostream.hpp>

#include "gb_core/cart.hpp"
#include "gb_core/defs.hpp"

#include "util/util.hpp"

/**
 * ROM with optional RAM
 */
struct ROM_Controller : public MemoryBankController {
    ROM_Controller(Cartridge_Constants::cart_type_t const& cart_type, std::vector<u8> const& rom, std::vector<u8> const& ram) :
    MemoryBankController(cart_type, rom, ram) {}

    u8 read(u16 offset) override {
        if(offset <= 0x7FFF) {
            return MemoryBankController::rom_data[offset];
        }
        else if(offset >= 0xA000 && offset <= 0xBFFF) {
            offset -= 0xA000;
            if(cart_type.RAM)
                return ram_data[offset];
            else
                return 0;
        }
        else {
            nowide::cerr << "ROM read out of bounds: " << as_hex(offset) << std::endl;
            return 0;
        }
    }

    void write(u16 offset, u8 data) override {
        if(offset >= 0xA000 && offset <= 0xBFFF) {
            offset -= 0xA000;
            if(cart_type.RAM) {
                MemoryBankController::ram_data[offset] = data;
            }
        }
        else {
            nowide::cerr << "ROM write out of bounds: " << as_hex(offset) << std::endl;
        }
    }
};