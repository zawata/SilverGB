#pragma once

#include "gb_core/cart.hpp"

#include "util/bit.hpp"
#include "util/util.hpp"

#include "mbc_base.hpp"

struct MBC1_Controller : public MBC1_Base {
    MBC1_Controller(Cartridge_Constants::cart_type_t cart_type, std::vector<u8> rom, std::vector<u8> ram) :
    MBC1_Base(cart_type, rom, ram) {}

    u8 read(u16 offset) override {
        if(bounded(offset, 0x0000_u16, 0x7FFF_u16)) {
            return MBC1_Base::read(offset);
        }
        else if(bounded(offset, 0xA000_u16, 0xBFFF_u16)) {
            return MBC1_Base::read(offset);
        }
        else {
            std::cerr << "MBC1: read out of bounds: " << offset << std::endl;
            return 0;
        }
    }

    void write(u16 offset, u8 data) override {
        if(bounded(offset, 0x0_u16, 0x1FFF_u16)) {
            MBC1_Base::set_ram_enable((data & 0xf) == 0xA);
        }
        else if(bounded(offset, 0x2000_u16, 0x3FFF_u16)) {
            u8 rom_bank_num = data & 0x1F;
            if(!rom_bank_num)
                rom_bank_num++; //correct rom bank from 0
            MBC1_Base::set_rom_bank(rom_bank_num);
        }
        else if(bounded(offset, 0x4000_u16, 0x5FFF_u16)) {
            addl_bank_num = data & 0x03;
        }
        else if(bounded(offset, 0x6000_u16, 0x7FFF_u16)) {
            bool set = Bit::test(data, 0);

            using namespace Cartridge_Constants;
            if(rom_data.size() >= Cartridge_Constants::ROM_SZ_1M) {
                set_rom_0_bank((set) ? (addl_bank_num << 6) : 0);
            } else if(ram_data.size() > Cartridge_Constants::RAM_SZ_8K) {
                set_ram_bank((set) ? addl_bank_num : 0);
            }
        }
        else if(bounded(offset, 0xA000_u16, 0xBFFF_u16)) {
            MBC1_Base::write(offset, data);
        }
        else {
            std::cerr << "MBC1: write out of bounds: " << offset << std::endl;
        }
    }

private:
    u8 addl_bank_num;
};