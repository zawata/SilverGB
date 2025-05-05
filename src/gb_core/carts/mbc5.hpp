#pragma once

#include <vector>

#include "util/bit.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

#include "cart.hpp"
#include "mbc_base.hpp"

/**
 * MBC5
 */
struct MBC5_Controller: public MBC1_Base {
    MBC5_Controller(
            Cartridge_Constants::cart_type_t const &cart_type, std::vector<u8> const &rom, std::vector<u8> const &ram) :
        MBC1_Base(cart_type, rom, ram) { }

    u8 read(u16 offset) override {
        if(bounded(offset, 0x0000_u16, 0xBFFF_u16)) {
            return MBC1_Base::read(offset);
        } else {
            LogError("MBC5") << "read out of bounds: " << as_hex(offset);
            return 0;
        }
    }

    void write(u16 offset, u8 data) override {
        if(bounded(offset, 0_u16, 0x1FFF_u16)) {
            MBC1_Base::set_ram_enable((data & 0xf) == 0xA);
        } else if(bounded(offset, 0x2000_u16, 0x2FFF_u16)) {
            u16 rom_bank_num = Bit::set_cond(data, 9, Bit::test(this->rom_bank, 9));
            MBC1_Base::set_rom_bank(rom_bank_num);
        } else if(bounded(offset, 0x2000_u16, 0x2FFF_u16)) {
            u16 rom_bank_num = Bit::set_cond(this->rom_bank, 9, Bit::test(data, 0));
            MBC1_Base::set_rom_bank(rom_bank_num);
        } else if(bounded(offset, 0x4000_u16, 0x5FFF_u16)) {
            // TODO: rumble
            if(Bit::test(data, 3)) {
                rumble_state = true;
            } else {
                rumble_state = false;
            }

            set_ram_bank(data & 0xF);
        } else if(bounded(offset, 0xA000_u16, 0xBFFF_u16)) {
            MBC1_Base::write(offset, data);
        } else {
            LogError("MBC5") << "write out of bounds: " << as_hex(offset);
        }
    }

private:
    bool rumble_state = false;
};