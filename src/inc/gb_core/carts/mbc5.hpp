#pragma once

#include "gb_core/cart.hpp"

/**
 * MBC5
 *
 * TODO: this
 */
struct MBC5_Controller : public MemoryBankController {
    MBC5_Controller(Cartridge_Constants::cart_type_t cart_type, std::vector<u8> rom, std::vector<u8> ram) :
    MemoryBankController(cart_type, rom, ram) {
        std::cerr << "MBC5 not yet implemented. Will probably crash now" << std::endl;
        if(cart_type.RAM)
            this->ram = ram;
    }

    u8 read(u16 offset) override {
        return 0;
    }

    void write(u16 offset, u8 data) override {}

private:
    std::vector<u8> ram;
};