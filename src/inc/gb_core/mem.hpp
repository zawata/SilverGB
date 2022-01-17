#pragma once

#include <vector>

#include "gb_core/defs.hpp"
#include "util/ints.hpp"

#define HIGH_RAM_SIZE 0x7f

#define WORK_RAM_BANK_SIZE 0x1000
#define WORK_RAM_BASE_SIZE 0x1000
#define DMG_WORK_RAM_SIZE  (WORK_RAM_BASE_SIZE + WORK_RAM_BANK_SIZE)
#define GBC_WORK_RAM_SIZE  (WORK_RAM_BASE_SIZE + (WORK_RAM_BANK_SIZE * 7))

#define DMG_VRAM_SIZE  0x3000
#define GBC_VRAM_SIZE  (DMG_VRAM_SIZE + DMG_VRAM_SIZE)

#define OAM_RAM_SIZE   0xA0

class Memory {
public:
    enum Interrupt {
        VBLANK_INT   = 1<<0,
        LCD_STAT_INT = 1<<1,
        TIMER_INT    = 1<<2,
        SERIAL_INT   = 1<<3,
        JOYPAD_INT   = 1<<4,
    };

    Memory(gb_device_t device);
    ~Memory();

    u8   read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    u8   read_vram(u16 loc, bool bypass = false, bool bypass_bank1 = false);
    void write_vram(u16 loc, u8 data, bool bypass = false, bool bypass_bank1 = false);

    u8   read_oam(u16 loc, bool bypass = false);
    void write_oam(u16 loc, u8 data);

    u8   read_ram(u16 loc);
    void write_ram(u16 loc, u8 data);

    u8   read_hram(u16 loc);
    void write_hram(u16 loc, u8 data);

    void request_interrupt(Interrupt i);


    struct io_registers_t {
        //Input
        // P1 //implemented in Input_Manager

        //Serial
        u8 SB; //Serial not supported
        u8 SC;

        //Timer
        // u8 DIV; // emulated
        u8 TIMA;
        u8 TMA;
        u8 TAC;

        //Interupt Flags
        u8 IF;

        // PPU
        u8 LCDC;
        u8 STAT;
        u8 SCY;
        u8 SCX;
        u8 LY;
        u8 LYC;
        u8 DMA;
        u8 BGP;
        u8 OBP0;
        u8 OBP1;
        u8 WY;
        u8 WX;
        u8 VBK;

        // GBC Registers
        u8 KEY1;
        // ROMEN // Not an actual register

        // HDMA
        u8 HDMA1;
        u8 HDMA2;
        u8 HDMA3;
        u8 HDMA4;
        u8 HDMA5;

        // Infrared
        u8 RP;

        // Color Palettes
        u8 BCPS;
        // u8 BCPD; // emulated
        u8 OCPS;
        // u8 OCPD; // emulated

        u8 SVBK;
        u8 IE;
    } registers;

private:
    gb_device_t device;

    u16 bank_offset;

    std::vector<u8> work_ram;
    std::vector<u8> high_ram;
    std::vector<u8> ppu_ram;
    std::vector<u8> oam_ram;
};