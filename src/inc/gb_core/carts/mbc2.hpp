#pragma once

#include "gb_core/cart.hpp"

#include "util/bit.hpp"

/**
 * MBC2
 *
 * TODO: this
 */
struct MBC2_Controller : public MemoryBankController {
    MBC2_Controller(Cartridge_Constants::cart_type_t cart_type, std::vector<u8> rom, std::vector<u8> ram) :
    MemoryBankController(cart_type, rom, ram) {
        std::cerr << "MBC2 not yet implemented. Will probably crash now" << std::endl;
        if(cart_type.RAM)
            this->ram = ram;

        ram_enable = false;
    }

    u8 read(u16 offset) override {
    }

    void write(u16 offset, u8 data) override {
    }


private:
    std::vector<u8> ram;

    bool ram_enable;
    u8 rom_bank_num = 0;
    //true = ram_banking, false = rom_banking
    bool mode_select = false;

    static const u16 ROM_BANK_SIZE = 0x4000;
    static const u16 RAM_BANK_SIZE = 0x2000;
};