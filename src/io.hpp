#pragma once

#include <vector>

#include "audio.hpp"
#include "cart.hpp"
#include "cfg.hpp"
#include "input.hpp"

#include "util/ints.hpp"

//Interupt Offsets
#define VBLANK_INT_OFFSET   0x40
#define LCD_STAT_INT_OFFSET 0x48
#define TIMER_INT_OFFSET    0x50
#define SERIAL_INT_OFFSET   0x58
#define JOYPAD_INT_OFFSET   0x60

#define HIGH_RAM_SIZE 0x7f

#define WORK_RAM_BANK_SIZE 0x1000
#define WORK_RAM_BASE_SIZE 0x1000
#define DMG_WORK_RAM_SIZE  (WORK_RAM_BASE_SIZE + WORK_RAM_BANK_SIZE)
#define GBC_WORK_RAM_SIZE  (WORK_RAM_BASE_SIZE + (WORK_RAM_BANK_SIZE * 7))

#define VRAM_BANK_SIZE      0x1800
#define DMG_VRAM_CHAR_SIZE  VRAM_BANK_SIZE
#define GBC_VRAM_CHAR_SIZE  (VRAM_BANK_SIZE + VRAM_BANK_SIZE)
#define VRAM_BACK_SIZE      0x1800

#define OAM_RAM_SIZE   0x9F

class IO_Bus {
public:
    enum Interrupt {
        VBLANK_INT   = 1<<0,
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

    u8   read_vram(u16 loc, bool bypass = false);
    void write_vram(u16 loc, u8 data);

    u8   read_oam(u16 loc, bool bypass = false);
    void write_oam(u16 loc, u8 data);

    u8   read_ram(u16 loc);
    void write_ram(u16 loc, u8 data);

    u8   read_hram(u16 loc);
    void write_hram(u16 loc, u8 data);

    void request_interrupt(int i);

    void      cpu_inc_DIV();
    void      cpu_inc_TIMA();
    u16       cpu_get_TAC_cs();

    Interrupt cpu_check_interrupts();
    void      cpu_unset_interrupt(Interrupt i);

    Sound_Controller *snd;
    Input_Manager    *input;

    struct io_registers_t {
        //Input
        u8 P1;

        //Serial
        u8 SB; //Serial not supported
        u8 SC;

        //Timer
        u8 DIV;
        u8 TIMA;
        u8 TMA;
        u8 TAC;

        //Interupt Flags
        u8 IF;

        //Video
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

private:
    Cartridge *cart;
    File_Interface *bootrom_file;
    Configuration *config;

    bool gbc_mode;
    bool bootrom_mode = false;

    u16 bank_offset;

    std::vector<u8> work_ram;
    std::vector<u8> high_ram;
    std::vector<u8> video_ram_char;
    std::vector<u8> video_ram_back;
    std::vector<u8> oam_ram;
};