#include <nowide/iostream.hpp>

#include "gb_core/mem.hpp"
#include "gb_core/io_reg.hpp"

#include "util/util.hpp"

#define MODE_HBLANK 0
#define MODE_VBLANK 1
#define MODE_OAM    2
#define MODE_VRAM   3
#define check_mode(x) ((registers.STAT & 0x3) == x)

Memory::Memory(bool gbc_mode) :
gbc_mode(gbc_mode) {
    if(gbc_mode) {
        work_ram.resize(GBC_WORK_RAM_SIZE);
        ppu_ram_char.resize(GBC_VRAM_CHAR_SIZE);
    }
    else {
        work_ram.resize(DMG_WORK_RAM_SIZE);
        ppu_ram_char.resize(DMG_VRAM_CHAR_SIZE);
    }

    high_ram.resize(HIGH_RAM_SIZE);
    ppu_ram_back.resize(VRAM_BACK_SIZE);
    oam_ram.resize(OAM_RAM_SIZE);
}

Memory::~Memory() {}

u8 Memory::read_reg(u8 loc) {
    switch(loc) {
    case P1_REG:
        //implemented in io.cpp
        break;

    //Serial Registers
    case SB_REG:
        return registers.SB & SB_READ_MASK;
    case SC_REG:
        return SC_DEFAULTS | (registers.SC & SC_READ_MASK);

    //Timer Registers
    case DIV_REG:
        //implemented in io.cpp
        break;
    case TIMA_REG:
        return registers.TIMA & TIMA_READ_MASK;
    case TMA_REG:
        return registers.TMA & TMA_READ_MASK;
    case TAC_REG:
        return TAC_DEFAULTS | (registers.TAC & TAC_READ_MASK);
    case IF_REG:
        return IF_DEFAULTS | (registers.IF & IF_READ_MASK);

    //Sound Registers
    case NR10_REG: case NR11_REG: case NR12_REG: case NR13_REG: case NR14_REG:
                   case NR21_REG: case NR22_REG: case NR23_REG: case NR24_REG:
    case NR30_REG: case NR31_REG: case NR32_REG: case NR33_REG: case NR34_REG:
                   case NR41_REG: case NR42_REG: case NR43_REG: case NR44_REG:
    case NR50_REG: case NR51_REG: case NR52_REG:
        //implemented in io.cpp
        break;

    case LCDC_REG:
        return registers.LCDC & LCDC_READ_MASK;
    case STAT_REG:
        return STAT_DEFAULTS | (registers.STAT & STAT_READ_MASK);
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
    case KEY1_REG:
        return registers.KEY1 & KEY1_READ_MASK;
    case ROMEN_REG:
        //implemented in io.cpp
        break;
    case HDMA1_REG:
    case HDMA2_REG:
    case HDMA3_REG:
    case HDMA4_REG:
    case HDMA5_REG:
    case RP_REG:
        return registers.RP   & RP_READ_MASK;
    case BCPS_REG:
    case BCPD_REG:
    case OCPS_REG:
    case OCPD_REG:
        break;
    case SVBK_REG:
        return registers.SVBK & SVBK_READ_MASK;
    case IE_REG:
        return registers.IE   & IE_READ_MASK;
    }

    nowide::cerr << "io/mem::reg_read miss:" << loc << std::endl;
    return 0;
}

void Memory::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case P1_REG:
        //implemented in io.cpp
        break;
    case SB_REG:
        registers.SB = data & SB_WRITE_MASK;
        return;
    case SC_REG:
        registers.SC = SC_DEFAULTS | (data & SC_WRITE_MASK);
        return;
    case DIV_REG:
        //implemented in io.cpp
        break;
    case TIMA_REG:
        registers.TIMA = data & TIMA_WRITE_MASK;
        return;
    case TMA_REG:
        registers.TMA = data & TMA_WRITE_MASK;
        return;
    case TAC_REG:
        registers.TAC = TAC_DEFAULTS | (data & TAC_WRITE_MASK);
        return;
    case IF_REG:
        registers.IF = IF_DEFAULTS | (data & IF_WRITE_MASK);
        return;

    //Sound Registers
    case NR10_REG: case NR11_REG: case NR12_REG: case NR13_REG: case NR14_REG:
                   case NR21_REG: case NR22_REG: case NR23_REG: case NR24_REG:
    case NR30_REG: case NR31_REG: case NR32_REG: case NR33_REG: case NR34_REG:
                   case NR41_REG: case NR42_REG: case NR43_REG: case NR44_REG:
    case NR50_REG: case NR51_REG: case NR52_REG:
        //implemented in io.cpp
        break;

#if _MSC_VER && !__INTEL_COMPILER
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x34: case 0x35: case 0x36: case 0x37:
    case 0x38: case 0x39: case 0x3A: case 0x3B:
    case 0x3C: case 0x3D: case 0x3E: case 0x3F:
#else
    //this seems to be supported on CLANG and GCC but not MSVC
    // if MSVC does support this I'll keep it because it's so much cleaner
    case 0x30 ... 0x3F:
#endif
        //implemented in io.cpp
        break;
    case LCDC_REG:
        registers.LCDC = data & LCDC_WRITE_MASK;
        return;
    case STAT_REG:
        registers.STAT = STAT_DEFAULTS | (data & STAT_WRITE_MASK);
        return;
    case SCY_REG:
        registers.SCY  = data & SCY_WRITE_MASK;
        return;
    case SCX_REG:
        registers.SCX  = data & SCX_WRITE_MASK;
        return;
    case LY_REG:
        registers.LY   = data & LY_WRITE_MASK;
        return;
    case LYC_REG:
        registers.LYC  = data & LYC_WRITE_MASK;
        return;
    case DMA_REG:
        registers.DMA  = data & DMA_WRITE_MASK;
        return;
    case BGP_REG:
        registers.BGP  = data & BGP_WRITE_MASK;
        return;
    case OBP0_REG:
        registers.OBP0 = data & OBP0_WRITE_MASK;
        return;
    case OBP1_REG:
        registers.OBP1 = data & OBP1_WRITE_MASK;
        return;
    case WY_REG:
        registers.WY   = data & WY_WRITE_MASK;
        return;
    case WX_REG:
        registers.WX   = data & WX_WRITE_MASK;
        return;
    case VBK_REG:
        if(gbc_mode) {
            registers.VBK = data & VBK_WRITE_MASK;
        }
        return;
    case KEY1_REG:
        registers.KEY1 = data & KEY1_WRITE_MASK;
        return;
    case ROMEN_REG:
        //implemented in io.cpp
        break;
    case HDMA1_REG:
        registers.HDMA1 = data & HDMA1_WRITE_MASK;
        return;
    case HDMA2_REG:
        registers.HDMA2 = data & HDMA2_WRITE_MASK;
        return;
    case HDMA3_REG:
        registers.HDMA3 = data & HDMA3_WRITE_MASK;
        return;
    case HDMA4_REG:
        registers.HDMA4 = data & HDMA4_WRITE_MASK;
        return;
    case HDMA5_REG:
        registers.HDMA5 = data & HDMA5_WRITE_MASK;
        return;
    case RP_REG:
        registers.RP = data & RP_WRITE_MASK;
        return;
    case BCPS_REG:
    case BCPD_REG:
    case OCPS_REG:
    case OCPD_REG:
        break;
    case SVBK_REG:
        registers.SVBK = data & SVBK_WRITE_MASK;
        //value 1-7 is bank 1-7
        //value 0   is bank 1
        if(registers.SVBK == 0)
            bank_offset = registers.SVBK * WORK_RAM_BANK_SIZE;
        else
            bank_offset = WORK_RAM_BANK_SIZE;
        break;
    case IE_REG:
        registers.IE = data & IE_WRITE_MASK;
        return;
    }

    nowide::cerr << "io/mem::reg_write miss:" << loc << std::endl;
}

/**
 * Only the first half of the vram is banked so the simplest way to
 * store data and process requests to and from it IMO is to separate VRAM
 * into 2 parts, character data and background data.
 *
 * The character data exists entirely in the VRAM banks while the background
 * data is unbanked.
 *
 * If a request comes in less than the bank size then we check the vram bank vector
 * and if it's higher then it goes to the unbanked background vector
 *
 * OAM is always unbanked and is just a simple read/write operation;
 *
 * ==============================================================================
 * The bypass flag is used for debugging and allows checking OAM and VRAM outside
 * of VBLANK and HBLANK
 *
 * TODO: do the writes need a bypass too?
 **/
u8 Memory::read_vram(u16 offset, bool bypass) {
    if(check_mode(MODE_VRAM) && !bypass) return 0xFF;

    if(offset >= 0x8000 && offset <= 0x9FFF) {
        offset -= 0x8000;

        if(offset < VRAM_BANK_SIZE) {
            if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
                return ppu_ram_char[VRAM_BANK_SIZE + offset];
            }
            else {
                return ppu_ram_char[offset];
            }
        }
        else {
            return ppu_ram_back[offset-VRAM_BANK_SIZE];
        }
    }
    else {
        nowide::cerr << "vram read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void Memory::write_vram(u16 offset, u8 data) {
    if(offset >= 0x8000 && offset <= 0x9FFF) {
        offset -= 0x8000;

        if(offset < VRAM_BANK_SIZE) {
            if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
                ppu_ram_char[VRAM_BANK_SIZE + offset] = data;
            }
            else {
                ppu_ram_char[offset] = data;
            }
        }
        else {
            ppu_ram_back[offset-VRAM_BANK_SIZE] = data;
        }
    }
    else {
        nowide::cerr << "vram write OOB: " << as_hex(offset) << std::endl;
    }
}

u8 Memory::read_oam(u16 offset, bool bypass) {
    if((check_mode(MODE_OAM) || check_mode(MODE_VRAM)) && !bypass) return 0xFF;

    if(offset >= 0xFE00 && offset <= 0xFE9F) {
        offset -= 0xFE00;

        return oam_ram[offset];
    }
    else {
        nowide::cerr << "oam read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void Memory::write_oam(u16 offset, u8 data) {
    if(offset >= 0xFE00 && offset <= 0xFE9F) {
        offset -= 0xFE00;

        oam_ram[offset] = data;
    }
    else {
        nowide::cerr << "oam write OOB: " << as_hex(offset) << std::endl;
    }
}

u8 Memory::read_ram(u16 offset) {
    if(offset >= 0xC000 && offset < 0xE000) {
        offset -= 0xC000;

        if(offset < 0x1000) {
            return work_ram[offset];
        } else {
            if(gbc_mode) {
                return work_ram[offset + bank_offset];
            } else {
                return work_ram[offset];
            }
        }
    }
    else {
        nowide::cerr << "ram read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void Memory::write_ram(u16 offset, u8 data) {
    if(offset >= 0xC000 && offset < 0xE000) {
        offset -= 0xC000;

        if(offset < 0x1000) {
            work_ram[offset] = data;
        } else {
            if(gbc_mode) {
                work_ram[offset + bank_offset] = data;
            } else {
                work_ram[offset] = data;
            }
        }
    }
    else {
        nowide::cerr << "ram write OOB: " << as_hex(offset) << std::endl;
    }
}

u8 Memory::read_hram(u16 offset) {
    if(offset >= 0xFF80 && offset < 0xFFFF) {
        offset -= 0xFF80;

        return high_ram[offset];
    }
    else {
        nowide::cerr << "hram read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void Memory::write_hram(u16 offset, u8 data) {
    if(offset >= 0xFF80 && offset < 0xFFFF) {
        offset -= 0xFF80;

        high_ram[offset] = data;
    }
    else {
        nowide::cerr << "hram write OOB: " << as_hex(offset) << std::endl;
    }
}

/**
 * Interface Routines
 */

// This doesn't sound liek a memory function but all it does is set the relevant IF flag so here's the best place to do it
void Memory::request_interrupt(Memory::Interrupt i) {
    registers.IF |= i;
}