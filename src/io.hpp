#pragma once

#include "audio.hpp"
#include "cart.hpp"
#include "cfg.hpp"
#include "input.hpp"
#include "video.hpp"

#include "util/ints.hpp"

//Interupt Offsets
#define V_BLANK_INT_OFFSET  0x40
#define LCD_STAT_INT_OFFSET 0x48
#define TIMER_INT_OFFSET    0x50
#define SERIAL_INT_OFFSET   0x58
#define JOYPAD_INT_OFFSET   0x60

#define HIGH_RAM_SIZE 0x7f

#define WORK_RAM_BANK_SIZE 0x1000
#define WORK_RAM_BASE_SIZE 0x1000
#define DMG_WORK_RAM_SIZE  (WORK_RAM_BASE_SIZE + WORK_RAM_BANK_SIZE)
#define GBC_WORK_RAM_SIZE  (WORK_RAM_BASE_SIZE + (WORK_RAM_BANK_SIZE * 7))

class IO_Bus {
public:
    enum Interrupt {
        V_BLANK_INT  = 1<<0,
        LCD_STAT_INT = 1<<1,
        TIMER_INT    = 1<<2,
        SERIAL_INT   = 1<<3,
        JOYPAD_INT   = 1<<4,
    };

    IO_Bus(Cartridge *cart, Configuration *config, bool gbc_mode);
    ~IO_Bus();

    u8   read(u16 offset);
    void write(u16 offset, u8 data);

    u8   read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    u8   read_ram(u8 loc);
    void write_ram(u8 loc, u8 data);

    u8   read_hram(u8 loc);
    void write_hram(u8 loc, u8 data);

    void      cpu_inc_DIV();
    void      cpu_inc_TIMA();
    u16       cpu_get_TAC_cs();

    Interrupt cpu_check_interrupts();
    void      cpu_unset_interrupt(Interrupt i);

private:
    Cartridge *cart;
    File_Interface *boot_rom_file;
    Configuration *config;

    bool gbc_mode;
    bool bootrom_mode;

    Video_Controller *vpu;
    Sound_Controller *snd;
    Input_Manager    *input;

    u16 bank_offset;

    std::vector<u8> work_ram;
    std::vector<u8> high_ram;

    struct __registers {
        u8 P1;
//        u8 SB; Serial not supported
//        u8 SC;
        u8 DIV;
        u8 TIMA;
        u8 TMA;
        u8 TAC;
        u8 IF;
        u8 LCDC;
        u8 STAT;
        u8 SCY;
        u8 SCX;
        u8 LY;
        u8 LYC;
        u8 DMA;
        u8 BGP;
        u8 OBP0;
        u8 OPB1;
        u8 WY;
        u8 WX;
/* GBC Registers
        u8 KEY1;
        u8 HDMA1;
        u8 HDMA2;
        u8 HDMA3;
        u8 HDMA4;
        u8 HDMA5;
        u8 RP;
        u8 BCPS;
        u8 BCPD;
        u8 OCPS;
        u8 OCPD;
*/
        u8 SVBK;
        u8 IE;
    } registers;
};