#include "mem.hpp"

#include "util/bit.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

#include "defs.hpp"

#define MODE_HBLANK   0
#define MODE_VBLANK   1
#define MODE_OAM      2
#define MODE_VRAM     3
#define check_mode(x) ((registers.STAT & 0x3) == x)

Memory::Memory(gb_device_t device, bool bootrom_enabled) :
    device(device) {
    if(dev_is_GBC(device)) {
        work_ram.resize(GBC_WORK_RAM_SIZE);
        ppu_ram.resize(GBC_VRAM_SIZE);
    } else {
        work_ram.resize(DMG_WORK_RAM_SIZE);
        ppu_ram.resize(DMG_VRAM_SIZE);
    }

    high_ram.resize(HIGH_RAM_SIZE);
    oam_ram.resize(OAM_RAM_SIZE);

    if(!bootrom_enabled) {
        if(dev_is_GBC(device)) {
            memcpy(oam_ram.data(), oam_ram_CGB_initial_state, oam_ram.size());
            memcpy(ppu_ram.data(), ppu_ram_CGB_initial_state, ppu_ram.size());
            memcpy(high_ram.data(), high_ram_CGB_initial_state, high_ram.size());
            memcpy(work_ram.data(), work_ram_CGB_initial_state, work_ram.size());
        }

        // TODO:
        registers = {};
    }
}

Memory::~Memory() { }

u8 Memory::read_reg(u8 loc) {
    switch(loc) {
    case P1_REG:
        // implemented in io.cpp
        break;

    // Serial Registers
    case SB_REG: return registers.SB & SB_READ_MASK;
    case SC_REG: return SC_DEFAULTS | (registers.SC & SC_READ_MASK);

    // Timer Registers
    case DIV_REG:
        // implemented in io.cpp
        break;
    case TIMA_REG: return registers.TIMA & TIMA_READ_MASK;
    case TMA_REG:  return registers.TMA & TMA_READ_MASK;
    case TAC_REG:  return TAC_DEFAULTS | (registers.TAC & TAC_READ_MASK);
    case IF_REG:   return IF_DEFAULTS | (registers.IF & IF_READ_MASK);

    // Sound Registers
    case NR10_REG:
    case NR11_REG:
    case NR12_REG:
    case NR13_REG:
    case NR14_REG:
    case NR21_REG:
    case NR22_REG:
    case NR23_REG:
    case NR24_REG:
    case NR30_REG:
    case NR31_REG:
    case NR32_REG:
    case NR33_REG:
    case NR34_REG:
    case NR41_REG:
    case NR42_REG:
    case NR43_REG:
    case NR44_REG:
    case NR50_REG:
    case NR51_REG:
    case NR52_REG:
        // implemented in io.cpp
        break;

    case LCDC_REG: return registers.LCDC & LCDC_READ_MASK;
    case STAT_REG: return STAT_DEFAULTS | (registers.STAT & STAT_READ_MASK);
    case SCY_REG:  return registers.SCY & SCY_READ_MASK;
    case SCX_REG:  return registers.SCX & SCX_READ_MASK;
    case LY_REG:   return registers.LY & LY_READ_MASK;
    case LYC_REG:  return registers.LYC & LYC_READ_MASK;
    case DMA_REG:  return registers.DMA & DMA_READ_MASK;
    case BGP_REG:  return registers.BGP & BGP_READ_MASK;
    case OBP0_REG: return registers.OBP0 & OBP0_READ_MASK;
    case OBP1_REG: return registers.OBP1 & OBP1_READ_MASK;
    case WY_REG:   return registers.WY & WY_READ_MASK;
    case WX_REG:   return registers.WX & WX_READ_MASK;
    case VBK_REG:  return VBK_DEFAULTS | (registers.VBK & VBK_READ_MASK);
    case KEY0_REG: return registers.KEY0 & KEY0_READ_MASK;
    case KEY1_REG: return registers.KEY1 & KEY1_READ_MASK;
    case ROMEN_REG:
        // implemented in io.cpp
        break;
    case HDMA1_REG: return HDMA1_DEFAULTS;
    case HDMA2_REG: return HDMA2_DEFAULTS;
    case HDMA3_REG: return HDMA3_DEFAULTS;
    case HDMA4_REG: return HDMA4_DEFAULTS;
    case HDMA5_REG: return registers.HDMA5 & HDMA5_READ_MASK;
    case RP_REG:    return registers.RP & RP_READ_MASK;
    case BCPS_REG:  return registers.BCPS & BCPS_READ_MASK;
    case BCPD_REG:
        // implented in io.cpp
        break;
    case OCPS_REG: return registers.OCPS & OCPS_READ_MASK;
    case OCPD_REG:
        // implented in io.cpp
        break;
    case SVBK_REG: return registers.SVBK & SVBK_READ_MASK;
    case IE_REG:   return registers.IE & IE_READ_MASK;
    }

    LogError("Memory") << "reg_read OOB: " << loc;
    return 0;
}

void Memory::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case P1_REG:
        // implemented in io.cpp
        break;
    case SB_REG: registers.SB = data & SB_WRITE_MASK; return;
    case SC_REG: registers.SC = SC_DEFAULTS | (data & SC_WRITE_MASK); return;
    case DIV_REG:
        // implemented in io.cpp
        break;
    case TIMA_REG: registers.TIMA = data & TIMA_WRITE_MASK; return;
    case TMA_REG:  registers.TMA = data & TMA_WRITE_MASK; return;
    case TAC_REG:  registers.TAC = TAC_DEFAULTS | (data & TAC_WRITE_MASK); return;
    case IF_REG:   registers.IF = IF_DEFAULTS | (data & IF_WRITE_MASK); return;

    // Sound Registers
    case NR10_REG:
    case NR11_REG:
    case NR12_REG:
    case NR13_REG:
    case NR14_REG:
    case NR20_REG:
    case NR21_REG:
    case NR22_REG:
    case NR23_REG:
    case NR24_REG:
    case NR30_REG:
    case NR31_REG:
    case NR32_REG:
    case NR33_REG:
    case NR34_REG:
    case NR40_REG:
    case NR41_REG:
    case NR42_REG:
    case NR43_REG:
    case NR44_REG:
    case NR50_REG:
    case NR51_REG:
    case NR52_REG:
        // implemented in io.cpp
        break;

#if _MSC_VER && !__INTEL_COMPILER
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
#else
    // this seems to be supported on CLANG and GCC but not MSVC
    //  if MSVC does support this I'll keep it because it's so much cleaner
    case 0x30 ... 0x3F:
#endif
        // implemented in io.cpp
        break;
    case LCDC_REG: registers.LCDC = data & LCDC_WRITE_MASK; return;
    case STAT_REG: registers.STAT = STAT_DEFAULTS | (data & STAT_WRITE_MASK); return;
    case SCY_REG:  registers.SCY = data & SCY_WRITE_MASK; return;
    case SCX_REG:  registers.SCX = data & SCX_WRITE_MASK; return;
    case LY_REG:   registers.LY = data & LY_WRITE_MASK; return;
    case LYC_REG:  registers.LYC = data & LYC_WRITE_MASK; return;
    case DMA_REG:  registers.DMA = data & DMA_WRITE_MASK; return;
    case BGP_REG:  registers.BGP = data & BGP_WRITE_MASK; return;
    case OBP0_REG: registers.OBP0 = data & OBP0_WRITE_MASK; return;
    case OBP1_REG: registers.OBP1 = data & OBP1_WRITE_MASK; return;
    case WY_REG:   registers.WY = data & WY_WRITE_MASK; return;
    case WX_REG:   registers.WX = data & WX_WRITE_MASK; return;
    case VBK_REG:
        if(dev_is_GBC(device)) {
            registers.VBK = data & VBK_WRITE_MASK;
        }
        return;
    case KEY0_REG: registers.KEY0 = data & KEY0_WRITE_MASK; return;
    case KEY1_REG:
        if(Bit::test(data, 0)) {
            LogDebug("Memory") << "Speed Switch requested";
        }
        registers.KEY1 = data & KEY1_WRITE_MASK;
        return;
    case ROMEN_REG:
        // implemented in io.cpp
        break;
    case HDMA1_REG: registers.HDMA1 = data & HDMA1_WRITE_MASK; return;
    case HDMA2_REG:
        if(data > 0xE0) {
            Bit::reset(data, 6);
        }
        registers.HDMA2 = data & HDMA2_WRITE_MASK;
        return;
    case HDMA3_REG: registers.HDMA3 = data & HDMA3_WRITE_MASK; return;
    case HDMA4_REG: registers.HDMA4 = data & HDMA4_WRITE_MASK; return;
    case HDMA5_REG: registers.HDMA5 = data & HDMA5_WRITE_MASK; return;
    case RP_REG:    registers.RP = data & RP_WRITE_MASK; return;
    case BCPS_REG:  registers.BCPS = data & BCPS_WRITE_MASK; return;
    case BCPD_REG:
        // implemented in io.cpp
        break;
    case OCPS_REG: registers.OCPS = data & OCPS_WRITE_MASK; return;
    case OCPD_REG:
        // implemented in io.cpp
        break;
    case SVBK_REG: registers.SVBK = data & SVBK_WRITE_MASK; return;
    case IE_REG:   registers.IE = data & IE_WRITE_MASK; return;
    }

    LogError("Memory") << "reg_write OOB:" << as_hex(loc);
}

/**
 * VRAM
 **/
u8 Memory::read_vram(u16 offset, bool bypass, bool bypass_bank1) {
    DebugCheck(bounded(offset, VIDEO_RAM_START, VIDEO_RAM_END)) << "read_vram OOB: " << as_hex(offset);

    if(check_mode(MODE_VRAM) && !bypass) {
        return 0xFF;
    }

    offset -= VIDEO_RAM_START;

    if(bypass) {
        return ppu_ram[(DMG_VRAM_SIZE * (bypass_bank1 ? 1 : 0)) + offset];
    }

    if(dev_is_GBC(device) && (registers.VBK & VBK_READ_MASK)) {
        return ppu_ram[DMG_VRAM_SIZE + offset];
    } else {
        return ppu_ram[offset];
    }
}

void Memory::write_vram(u16 offset, u8 data, bool bypass, bool bypass_bank1) {
    DebugCheck(bounded(offset, VIDEO_RAM_START, VIDEO_RAM_END)) << "write_vram OOB: " << as_hex(offset);

    offset -= VIDEO_RAM_START;

    if(bypass) {
        ppu_ram[(DMG_VRAM_SIZE * (bypass_bank1 ? 1 : 0)) + offset] = data;
        return;
    }

    if(dev_is_GBC(device) && !get_dmg_compat_mode() && (registers.VBK & VBK_WRITE_MASK)) {
        ppu_ram[DMG_VRAM_SIZE + offset] = data;
    } else {
        ppu_ram[offset] = data;
    }
}

/**
 * OAM
 **/
u8 Memory::read_oam(u16 offset, bool bypass) {
    DebugCheck(bounded(offset, OBJECT_RAM_START, OBJECT_RAM_END)) << "read_oam OOB: " << as_hex(offset);

    if((check_mode(MODE_OAM) || check_mode(MODE_VRAM)) && !bypass) {
        return 0xFF;
    }

    offset -= OBJECT_RAM_START;
    return oam_ram[offset];
}

void Memory::write_oam(u16 offset, u8 data) {
    DebugCheck(bounded(offset, OBJECT_RAM_START, OBJECT_RAM_END)) << "write_oam OOB: " << as_hex(offset);

    offset -= OBJECT_RAM_START;
    oam_ram[offset] = data;
}

/**
 * RAM
 **/
u8 Memory::read_ram(u16 offset) {
    DebugCheck(bounded(offset, WORK_RAM_BANK0_START, WORK_RAM_BANK1_END)) << "read_ram OOB: " << as_hex(offset);

    if(offset <= WORK_RAM_BANK0_END) {
        offset -= WORK_RAM_BANK0_START;
        return work_ram[offset];
    } else {
        offset -= WORK_RAM_BANK0_START;
        if(dev_is_GBC(device)) {
            offset -= WORK_RAM_BANK_SIZE;
            // value 1-7 is bank 1-7
            // value 0   is bank 1
            u16 bank_mult = ((registers.SVBK == 0) ? 1 : registers.SVBK) & SVBK_READ_MASK;
            return work_ram[offset + (bank_mult * WORK_RAM_BANK_SIZE)];
        } else {
            return work_ram[offset];
        }
    }
}

void Memory::write_ram(u16 offset, u8 data) {
    DebugCheck(bounded(offset, WORK_RAM_BANK0_START, WORK_RAM_BANK1_END)) << "write_ram OOB: " << as_hex(offset);

    if(offset <= WORK_RAM_BANK0_END) {
        offset -= WORK_RAM_BANK0_START;
        work_ram[offset] = data;
    } else {
        offset -= WORK_RAM_BANK0_START;
        if(dev_is_GBC(device)) {
            offset -= WORK_RAM_BANK_SIZE;
            // value 1-7 is bank 1-7
            // value 0   is bank 1
            u16 bank_mult = ((registers.SVBK == 0) ? 1 : registers.SVBK) & SVBK_READ_MASK;
            work_ram[offset + (bank_mult * WORK_RAM_BANK_SIZE)] = data;
        } else {
            work_ram[offset] = data;
        }
    }
}

/**
 * HRAM
 **/
u8 Memory::read_hram(u16 offset) {
    DebugCheck(bounded(offset, HIGH_RAM_START, HIGH_RAM_END)) << "read_hram OOB: " << as_hex(offset);

    offset -= HIGH_RAM_START;
    return high_ram[offset];
}

void Memory::write_hram(u16 offset, u8 data) {
    DebugCheck(bounded(offset, HIGH_RAM_START, HIGH_RAM_END)) << "write_hram OOB: " << as_hex(offset);

    offset -= HIGH_RAM_START;
    high_ram[offset] = data;
}

/**
 * Interface Routines
 */

// This doesn't sound like a memory function but all it does is set the relevant IF bit so here's the best place to do
// it
void Memory::request_interrupt(Memory::Interrupt i) { registers.IF |= i; }

void Memory::set_dmg_compat_mode(bool compat_mode) {
    LogDebug("Memory") << "set_dmg_compat_mode " << compat_mode;

    if(compat_mode) {
        Bit::set(&registers.KEY0, 2);
    } else {
        Bit::reset(&registers.KEY0, 2);
    }
}

bool Memory::get_dmg_compat_mode() { return Bit::test(registers.KEY0, 2); }
