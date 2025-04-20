#pragma once

#include <vector>

#include "gba_core/cart.hpp"

class IO_Bus {
public:
    explicit IO_Bus(Cartridge *cart, Silver::File *bios_file = nullptr);

    ~IO_Bus();

    void tick();

    u32  read(u32 offset, bool bypass = false);

    void write(u16 offset, u8 data);

    u8   read_reg(u8 loc);

    void write_reg(u8 loc, u8 data);

    u8   read_vram(u16 loc, bool bypass = false);

    void write_vram(u16 loc, u8 data);

    u8   read_oam(u16 loc, bool bypass = false);

    void write_oam(u16 loc, u8 data);

    u8   read_ram(u16 loc);

    void write_ram(u16 loc, u8 data);

    u8   read_hram(u16 loc);

    void write_hram(u16 loc, u8 data);

private:
    Cartridge      *cart;
    std::vector<u8> bootrom_buffer;

    bool            gbc_mode;
    bool            bootrom_mode = false;
};
