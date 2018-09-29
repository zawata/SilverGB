#pragma once

#include <vector>

#include "cfg.hpp"
#include "util/ints.hpp"

#define VRAM_BANK_SIZE 0x1800
#define DMG_VRAM_SIZE  0x2000
#define GBC_VRAM_SIZE  (DMG_VRAM_SIZE + VRAM_BANK_SIZE)

#define OAM_RAM_SIZE   0x9F

class Video_Controller {
public:
    Video_Controller();
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
    } registers;

    std::vector<u8> video_ram;
    std::vector<u8> oam_ram;
};