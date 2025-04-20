#pragma once

#include <vector>

#include "util/bit.hpp"
#include "util/log.hpp"

#include "cart.hpp"

/**
 * MBC2
 *
 * TODO: this
 */
struct MBC2_Controller: public MemoryBankController {
    MBC2_Controller(
            Cartridge_Constants::cart_type_t const &cart_type, std::vector<u8> const &rom, std::vector<u8> const &ram) :
        MemoryBankController(cart_type, rom, ram) {
        LogError("MBC2") << "MBC2 not yet implemented. Will probably crash now";
        if(cart_type.RAM) {
            this->ram = ram;
        }

        ram_enable = false;
    }

    u8   read(u16 offset) override { return 0; }

    void write(u16 offset, u8 data) override { }

private:
    std::vector<u8>  ram;

    bool             ram_enable;
    u8               rom_bank_num  = 0;
    // true = ram_banking, false = rom_banking
    bool             mode_select   = false;

    static const u16 ROM_BANK_SIZE = 0x4000;
    static const u16 RAM_BANK_SIZE = 0x2000;
};