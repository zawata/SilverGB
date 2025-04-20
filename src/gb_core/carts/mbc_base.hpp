#pragma once

#include <vector>

#include "util/log.hpp"
#include "util/util.hpp"

#include "cart.hpp"
#include "defs.hpp"

struct MBC1_Base: public MemoryBankController {
    MBC1_Base(
            Cartridge_Constants::cart_type_t const &cart_type, std::vector<u8> const &rom, std::vector<u8> const &ram) :
        MemoryBankController(cart_type, rom, ram) {
        ram_enable = false;
        ram_bank   = 0;
        rom_bank   = 0;
        rom_0_bank = 0;
    }

    virtual void set_rom_0_bank(u16 rom_bank) { this->rom_0_bank = rom_bank; }

    virtual void set_rom_bank(u16 rom_bank) { this->rom_bank = rom_bank; }

    virtual void set_ram_bank(u16 ram_bank) { this->ram_bank = ram_bank; }

    virtual void set_ram_enable(bool ram_enable = true) { this->ram_enable = ram_enable; }

    virtual u8   read(u16 offset) override {
        if(bounded(offset, CART_ROM_BANK0_START, CART_ROM_BANK0_END)) {
            u32 addr = (u32)offset + (rom_0_bank * ROM_BANK_SIZE);

            if(addr < rom_data.size()) {
                return rom_data[addr];
            } else {
                return 0;
            }
        } else if(bounded(offset, CART_ROM_BANK1_START, CART_ROM_BANK1_END)) {
            offset -= CART_ROM_BANK1_START;

            u32 addr = (u32)offset + (rom_bank * ROM_BANK_SIZE);

            if(addr < rom_data.size()) {
                return rom_data[addr];
            } else {
                return 0;
            }
        } else if(bounded(offset, CART_RAM_START, CART_RAM_END)) {
            offset -= CART_RAM_START;

            u32 addr = (u32)offset + (ram_bank * RAM_BANK_SIZE);

            if(cart_type.RAM && ram_enable && addr < ram_data.size()) {
                return ram_data[addr];
            } else {
                return 0;
            }
        } else {
            LogError("MBC1Base") << "read OOB: " << offset;
            return 0;
        }
    }

    virtual void write(u16 offset, u8 data) override {
        if(bounded(offset, CART_RAM_START, CART_RAM_END)) {
            offset -= CART_RAM_START;

            u32 addr = (u32)offset + (ram_bank * RAM_BANK_SIZE);

            if(cart_type.RAM && ram_enable && addr < ram_data.size()) {
                ram_data[addr] = data;
            }
        } else {
            LogError("MBC1Base") << "write OOB: " << offset;
        }
    }

protected:
    static const u16 ROM_BANK_SIZE = 0x4000;
    static const u16 RAM_BANK_SIZE = 0x2000;

    bool             ram_enable    = false;
    u16              ram_bank = 0, rom_bank = 0, rom_0_bank = 0;
};