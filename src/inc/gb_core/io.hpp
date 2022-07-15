#pragma once

#include <vector>

#include "gb_core/apu.hpp"
#include "gb_core/cart.hpp"
#include "gb_core/joy.hpp"
#include "gb_core/ppu.hpp"

#include "util/ints.hpp"

// Interupt Offsets
#define VBLANK_INT_OFFSET   0x40
#define LCD_STAT_INT_OFFSET 0x48
#define TIMER_INT_OFFSET    0x50
#define SERIAL_INT_OFFSET   0x58
#define JOYPAD_INT_OFFSET   0x60

using Interrupt = Memory::Interrupt;

class CPU;

class IO_Bus {
    friend CPU;

public:
    IO_Bus(Memory *mem, APU *apu, PPU *ppu, Joypad *joy, Cartridge *cart, gb_device_t device, Silver::File *bios_file);
    ~IO_Bus();

    u8   read(u16 offset, bool bypass = false);
    void write(u16 offset, u8 data);

    u8   read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    void dma_tick();
    void gdma_tick();
    void hdma_tick();

private:
    void gbc_dma_copy_block();

    Memory *mem;
    APU    *apu;
    PPU    *ppu;
    Joypad *joy;

    Cartridge      *cart;
    std::vector<u8> bootrom_buffer;
    bool            bootrom_mode = false;

    gb_device_t device;

    bool dma_start        = false;
    bool dma_start_active = false;
    bool dma_active       = false;
    bool gdma_start       = false;
    bool gdma_active      = false;
    bool hdma_start       = false;
    bool hdma_active      = false;
    bool hdma_can_copy    = false;
    u16  dma_tick_cnt     = 0;
    u16  dma_byte_cnt     = 0;
    u16  gdma_tick_cnt    = 0;

    u16 bank_offset;
    u16 div_cnt = 0;
};