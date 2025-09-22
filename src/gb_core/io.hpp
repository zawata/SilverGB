#pragma once

#include <optional>
#include <vector>

#include "util/types/primitives.hpp"

#include "apu.hpp"
#include "cart.hpp"
#include "joy.hpp"
#include "ppu.hpp"

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
    IO_Bus(Memory *mem, APU *apu, PPU *ppu, Joypad *joy, Cartridge *cart, gb_device_t device,
           const std::optional<std::shared_ptr<Silver::File>> &bootrom = std::nullopt);
    ~IO_Bus();

    u8   read(u16 offset, bool bypass = false);
    void write(u16 offset, u8 data);

    u8   read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    void dma_tick();
    void gdma_tick();
    void hdma_tick();

private:
    void            gbc_dma_copy_block();

    Memory         *mem;
    APU            *apu;
    PPU            *ppu;
    Joypad         *joy;

    Cartridge      *cart;
    std::vector<u8> bootrom_buffer;

    gb_device_t     device;
    bool            bootrom_mode = false;

    bool dma_start = false, dma_start_active = false, dma_active = false, gdma_start = false, gdma_active = false,
         hdma_start = false, hdma_active = false, hdma_can_copy = false;
    u16 dma_tick_cnt = 0, dma_byte_cnt = 0, gdma_tick_cnt = 0;

    u16 bank_offset;
    u16 div_cnt = 0;
};