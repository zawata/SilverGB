#pragma once

#include "cart.hpp"
#include "defs.hpp"

#include "util/log.hpp"
#include "util/util.hpp"

/**
 * ROM with optional RAM
 */
struct ROM_Controller : public MemoryBankController {
    ROM_Controller(Cartridge_Constants::cart_type_t const& cart_type, std::vector<u8> const& rom, std::vector<u8> const& ram) :
    MemoryBankController(cart_type, rom, ram) {}

    u8 read(u16 offset) override {
        if(offset <= CART_ROM_BANK1_END) {
            return MemoryBankController::rom_data[offset];
        }
        else if(offset >= CART_RAM_START && offset <= CART_RAM_END) {
            offset -= CART_RAM_START;
            if(cart_type.RAM)
                return ram_data[offset];
            else
                return 0;
        }
        else {
            LogError("ROM") << "read out of bounds: " << as_hex(offset);
            return 0;
        }
    }

    void write(u16 offset, u8 data) override {
        if(offset >= CART_RAM_START && offset <= CART_RAM_END) {
            offset -= CART_RAM_START;
            if(cart_type.RAM) {
                MemoryBankController::ram_data[offset] = data;
            }
        }
        else {
            LogError("ROM") << "write out of bounds: " << as_hex(offset);
        }
    }
};