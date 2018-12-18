#pragma once

#include <vector>

#include "cfg.hpp"
#include "util/ints.hpp"

#define VRAM_BANK_SIZE      0x1800
#define DMG_VRAM_CHAR_SIZE  VRAM_BANK_SIZE
#define GBC_VRAM_CHAR_SIZE  (VRAM_BANK_SIZE + VRAM_BANK_SIZE)
#define VRAM_BACK_SIZE      0x1800

#define OAM_RAM_SIZE   0x9F

class Video_Controller {
public:
    Video_Controller(bool gbc_mode);
    ~Video_Controller();

    u8 read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    u8 read_ram(u16 loc);
    void write_ram(u16 loc, u8 data);

    u8 read_oam(u16 loc);
    void write_oam(u16 loc, u8 data);

private:
    bool gbc_mode;
    struct __registers {
        u8 VBK;

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
    } registers;

    std::vector<u8> video_ram_char;
    std::vector<u8> video_ram_back;
    std::vector<u8> oam_ram;
};