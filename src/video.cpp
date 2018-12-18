#include "video.hpp"

#include "io_reg.hpp"

Video_Controller::Video_Controller(bool gbc_mode) {
    if(gbc_mode)
        video_ram_char.reserve(GBC_VRAM_CHAR_SIZE);
    else
        video_ram_char.reserve(DMG_VRAM_CHAR_SIZE);
    video_ram_back.reserve(VRAM_BACK_SIZE);
    oam_ram.reserve(OAM_RAM_SIZE);
}

Video_Controller::~Video_Controller() {}

u8 Video_Controller::read_reg(u8 loc) {
    switch(loc) {
    case LCDC_REG:
        return registers.LCDC & LCDC_READ_MASK;
    case STAT_REG:
        return registers.STAT & STAT_READ_MASK;
    case SCY_REG:
        return registers.SCY  & SCY_READ_MASK;
    case SCX_REG:
        return registers.SCX  & SCX_READ_MASK;
    case LY_REG:
        return registers.LY   & LY_READ_MASK;
    case LYC_REG:
        return registers.LYC  & LYC_READ_MASK;
    case DMA_REG:
        return registers.DMA  & DMA_READ_MASK;
    case BGP_REG:
        return registers.BGP  & BGP_READ_MASK;
    case OBP0_REG:
        return registers.OBP0 & OBP0_READ_MASK;
    case OBP1_REG:
        return registers.OBP1 & OBP1_READ_MASK;
    case WY_REG:
        return registers.WY   & WY_READ_MASK;
    case WX_REG:
        return registers.WX   & WX_READ_MASK;
    case VBK_REG:
        return registers.VBK  | 0xFE; //all bits but bit0 one are set to 1
    }
}
void Video_Controller::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case LCDC_REG:
        registers.LCDC = data & LCDC_WRITE_MASK;
        break;
    case STAT_REG:
        registers.STAT = data & STAT_WRITE_MASK;
        break;
    case SCY_REG:
        registers.SCY  = data & SCY_WRITE_MASK;
        break;
    case SCX_REG:
        registers.SCX  = data & SCX_WRITE_MASK;
        break;
    case LY_REG:
        registers.LY   = data & LY_WRITE_MASK;
        break;
    case LYC_REG:
        registers.LYC  = data & LYC_WRITE_MASK;
        break;
    case DMA_REG:
        registers.DMA  = data & DMA_WRITE_MASK;
        break;
    case BGP_REG:
        registers.BGP  = data & BGP_WRITE_MASK;
        break;
    case OBP0_REG:
        registers.OBP0 = data & OBP0_WRITE_MASK;
        break;
    case OBP1_REG:
        registers.OBP1 = data & OBP1_WRITE_MASK;
        break;
    case WY_REG:
        registers.WY   = data & WY_WRITE_MASK;
        break;
    case WX_REG:
        registers.WX   = data & WX_WRITE_MASK;
        break;
    case VBK_REG:
        if(gbc_mode) {
            registers.VBK = data & VBK_WRITE_MASK;
        }
        break;
    }
}

/**
 * Only the first half of the vram is banked so the simplest way to
 * store data and process requests to and from it IMO is to separate VRAM
 * into 2 parts, character data and background data.
 *
 * The character data exists entirely in the VRAM banks while the background
 * data is unbanked.
 *
 * If a request comes in less than the bank limit then we check the vram bank vector
 * and if it's higher then it goes to the unbanked background vector
 *
 **/
u8 Video_Controller::read_ram(u16 loc) {
    if(loc < VRAM_BANK_SIZE) {
        if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
            return video_ram_char[VRAM_BANK_SIZE + loc];
        }
        else {
            return video_ram_char[loc];
        }
    }
    else {
        return video_ram_back[loc-VRAM_BANK_SIZE];
    }
}

void Video_Controller::write_ram(u16 loc, u8 data) {
    if(loc < VRAM_BANK_SIZE) {
        if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
            video_ram_char[VRAM_BANK_SIZE + loc] = data;
        }
        else {
            video_ram_char[loc] = data;
        }
    }
    else {
        video_ram_back[loc-VRAM_BANK_SIZE] = data;
    }
}

u8 Video_Controller::read_oam(u16 loc) {
    return oam_ram[loc];
}

void Video_Controller::write_oam(u16 loc, u8 data) {
    oam_ram[loc] = data;
}