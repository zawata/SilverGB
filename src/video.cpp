#include "video.hpp"

#include "io_reg.hpp"

Video_Controller::Video_Controller() {
    if(gbc_mode)
        video_ram.reserve(GBC_VRAM_SIZE);
    else
        video_ram.reserve(DMG_VRAM_SIZE);
    oam_ram.reserve(OAM_RAM_SIZE);
}

Video_Controller::~Video_Controller() {}

u8 Video_Controller::read_reg(u8 loc) {
    switch(loc) {
        case VBK_REG:
            return registers.VBK & VBK_READ_MASK;
    }
}
void Video_Controller::write_reg(u8 loc, u8 data) {
    switch(loc) {
        case VBK_REG:
            if(gbc_mode) {
                registers.VBK = data & VBK_READ_MASK;
            }
    }
}

u8 Video_Controller::read_ram(u16 loc) {
    if(loc < VRAM_BANK_SIZE) {
        if(gbc_mode && (registers.VBK & 1)) {
            return video_ram[VRAM_BANK_SIZE + loc];
        }
        else {
            return video_ram[loc];
        }
    }
    else {
        return video_ram[2 * VRAM_BANK_SIZE + loc];
    }
}

void Video_Controller::write_ram(u16 loc, u8 data) {
    if(loc < VRAM_BANK_SIZE) {
        if(gbc_mode && (registers.VBK & 1)) {
            video_ram[VRAM_BANK_SIZE + loc] = data;
        }
        else {
            video_ram[loc] = data;
        }
    }
    else {
        video_ram[2 * VRAM_BANK_SIZE + loc] = data;
    }
}

u8 Video_Controller::read_oam(u16 loc) {
    return oam_ram[loc];
}

void Video_Controller::write_oam(u16 loc, u8 data) {
    oam_ram[loc] = data;
}