#pragma once

#include <nowide/iostream.hpp>

#include "gb_core/cart.hpp"

#include "util/util.hpp"

struct MBC1_Base : public MemoryBankController {
    MBC1_Base(Cartridge_Constants::cart_type_t const& cart_type, std::vector<u8> const& rom, std::vector<u8> const& ram) :
    MemoryBankController(cart_type, rom, ram) {

        ram_enable = false;
        ram_bank = 0;
        rom_bank = 0;
        rom_0_bank = 0;
    }

    virtual void set_rom_0_bank(u16 rom_bank) {
        this->rom_0_bank = rom_bank;
    }

    virtual void set_rom_bank(u16 rom_bank) {
        this->rom_bank = rom_bank;
    }

    virtual void set_ram_bank(u16 ram_bank) {
        this->ram_bank = ram_bank;
    }

    virtual void set_ram_enable(bool ram_enable = true) {
        this->ram_enable = ram_enable;
    }

    virtual u8 read(u16 offset) override {
        if(bounded(offset, 0_u16, 0x3FFF_u16)) {
            u32 addr = (u32)offset + (rom_0_bank * ROM_BANK_SIZE);

            if(addr < rom_data.size()) {
                return rom_data[addr];
            }
            else {
                return 0;
            }
            return rom_data[offset];
        }
        else if(bounded(offset, 0x4000_u16, 0x7FFF_u16)) {
            offset -= 0x4000;

            u32 addr = (u32)offset + (rom_bank * ROM_BANK_SIZE);

            if(addr < rom_data.size()) {
                return rom_data[addr];
            }
            else {
                return 0;
            }
        }
        else if(bounded(offset, 0xA000_u16, 0xBFFF_u16)) {
            offset -= 0xA000;

            u32 addr = (u32)offset + (ram_bank * RAM_BANK_SIZE);

            if(cart_type.RAM && ram_enable && addr < ram_data.size()) {
                return ram_data[addr];
            }
            else {
                return 0;
            }
        }
        else {
            nowide::cerr << "read out of bounds: " << offset << std::endl;
            return 0;
        }
    }

    virtual void write(u16 offset, u8 data) override {
        if(bounded(offset, 0xA000_u16, 0xBFFF_u16)) {
            offset -= 0xA000;

            u32 addr = (u32)offset + (ram_bank * RAM_BANK_SIZE);

            if(cart_type.RAM && ram_enable && addr < ram_data.size()) {
                ram_data[addr] = data;
            }
        }
        else {
            nowide::cerr << "write out of bounds: " << offset << std::endl;
        }
    }

protected:
    static const u16 ROM_BANK_SIZE = 0x4000;
    static const u16 RAM_BANK_SIZE = 0x2000;

    bool ram_enable = false;
    u16 ram_bank = 0,
        rom_bank = 0,
        rom_0_bank = 0;
};