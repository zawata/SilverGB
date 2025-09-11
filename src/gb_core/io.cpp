#include "io.hpp"

#include "util/bit.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

#include "defs.hpp"
#include "mem.hpp"

#define reg(X)            (mem->registers.X)

#define MODE_HBLANK       0
#define MODE_VBLANK       1
#define MODE_OAM          2
#define MODE_VRAM         3
#define check_ppu_mode(x) ((reg(STAT) & 0x3) == x)

IO_Bus::IO_Bus(
        Memory *mem, APU *apu, PPU *ppu, Joypad *joy, Cartridge *cart, gb_device_t device,
        Silver::File *bios_file = nullptr) :
    mem(mem), apu(apu), ppu(ppu), joy(joy), cart(cart), device(device), bootrom_mode(bios_file != nullptr) {
    if(bios_file == nullptr) {
        reg(P1)             = default_reg_values[0][get_default_idx(device)];
        reg(SB)             = default_reg_values[1][get_default_idx(device)];
        reg(SC)             = default_reg_values[2][get_default_idx(device)];
        reg(DIV)            = default_reg_values[3][get_default_idx(device)];
        reg(TIMA)           = default_reg_values[4][get_default_idx(device)];
        reg(TMA)            = default_reg_values[5][get_default_idx(device)];
        reg(TAC)            = default_reg_values[6][get_default_idx(device)];
        reg(IF)             = default_reg_values[7][get_default_idx(device)];
        apu->registers.NR10 = default_reg_values[8][get_default_idx(device)];
        apu->registers.NR11 = default_reg_values[9][get_default_idx(device)];
        apu->registers.NR12 = default_reg_values[10][get_default_idx(device)];
        apu->registers.NR13 = default_reg_values[11][get_default_idx(device)];
        apu->registers.NR14 = default_reg_values[12][get_default_idx(device)];
        apu->registers.NR21 = default_reg_values[13][get_default_idx(device)];
        apu->registers.NR22 = default_reg_values[14][get_default_idx(device)];
        apu->registers.NR23 = default_reg_values[15][get_default_idx(device)];
        apu->registers.NR24 = default_reg_values[16][get_default_idx(device)];
        apu->registers.NR30 = default_reg_values[17][get_default_idx(device)];
        apu->registers.NR31 = default_reg_values[18][get_default_idx(device)];
        apu->registers.NR32 = default_reg_values[19][get_default_idx(device)];
        apu->registers.NR33 = default_reg_values[20][get_default_idx(device)];
        apu->registers.NR34 = default_reg_values[21][get_default_idx(device)];
        apu->registers.NR41 = default_reg_values[22][get_default_idx(device)];
        apu->registers.NR42 = default_reg_values[23][get_default_idx(device)];
        apu->registers.NR43 = default_reg_values[24][get_default_idx(device)];
        apu->registers.NR44 = default_reg_values[25][get_default_idx(device)];
        apu->registers.NR50 = default_reg_values[26][get_default_idx(device)];
        apu->registers.NR51 = default_reg_values[27][get_default_idx(device)];
        apu->registers.NR52 = default_reg_values[28][get_default_idx(device)];
        reg(LCDC)           = default_reg_values[29][get_default_idx(device)];
        reg(STAT)           = default_reg_values[30][get_default_idx(device)];
        reg(SCY)            = default_reg_values[31][get_default_idx(device)];
        reg(SCX)            = default_reg_values[32][get_default_idx(device)];
        reg(LY)             = default_reg_values[33][get_default_idx(device)];
        reg(LYC)            = default_reg_values[34][get_default_idx(device)];
        reg(DMA)            = default_reg_values[35][get_default_idx(device)];
        reg(BGP)            = default_reg_values[36][get_default_idx(device)];
        reg(OBP0)           = default_reg_values[37][get_default_idx(device)];
        reg(OBP1)           = default_reg_values[38][get_default_idx(device)];
        reg(WY)             = default_reg_values[39][get_default_idx(device)];
        reg(WX)             = default_reg_values[40][get_default_idx(device)];
        reg(KEY1)           = default_reg_values[41][get_default_idx(device)];
        reg(VBK)            = default_reg_values[42][get_default_idx(device)];
        reg(HDMA1)          = default_reg_values[43][get_default_idx(device)];
        reg(HDMA2)          = default_reg_values[44][get_default_idx(device)];
        reg(HDMA3)          = default_reg_values[45][get_default_idx(device)];
        reg(HDMA4)          = default_reg_values[46][get_default_idx(device)];
        reg(HDMA5)          = default_reg_values[47][get_default_idx(device)];
        reg(RP)             = default_reg_values[48][get_default_idx(device)];
        reg(BCPS)           = default_reg_values[49][get_default_idx(device)];
        reg(BCPD)           = default_reg_values[50][get_default_idx(device)];
        reg(OCPS)           = default_reg_values[51][get_default_idx(device)];
        reg(OCPD)           = default_reg_values[52][get_default_idx(device)];
        reg(SVBK)           = default_reg_values[53][get_default_idx(device)];
        reg(IE)             = default_reg_values[54][get_default_idx(device)];
    } else {
        bios_file->toVector(bootrom_buffer);
    }
}

IO_Bus::~IO_Bus() { }

u8 IO_Bus::read(u16 offset, bool bypass) {
    // TODO: this is technically not accurate
    // see future_work/failing_tests/dma/read_read
    if(dma_active && !bypass) {
        if(bounded(offset, HIGH_RAM_START, HIGH_RAM_END)) {
            return mem->read_hram(offset);
        } else {
            return 0xFF; // TODO:
        }
    }

    if(offset <= CART_ROM_BANK0_END) {
        // 16KB ROM bank 00
        if(bootrom_mode) {
            // the gameboy maps its bootrom data from 0x0000 to 0x00FF.
            // The gameboy color maps its bootrom data from 0x0000 to 0x08FF with a fallthrough from 0x100 to 0x1FF for
            // the cartridge header.
            if((offset <= GB_BOOTROM_END)
               || (dev_is_GBC(device) && bounded(offset, GBC_BOOTROM_START, GBC_BOOTROM_END))) {
                return bootrom_buffer[offset];
            }
        }

        return cart->read(offset);
    } else if(offset <= CART_ROM_BANK1_END) {
        // 16KB ROM Bank 01~NN
        return cart->read(offset); // cart will handle banking
    } else if(offset <= VIDEO_RAM_END) {
        // 8KB ppu RAM (VRAM)
        return mem->read_vram(offset);
    } else if(offset <= CART_RAM_END) {
        // 8KB External RAM
        return cart->read(offset);
    } else if(offset <= WORK_RAM_BANK0_END) {
        // 4KB Work RAM (WRAM) bank 0
        return mem->read_ram(offset);
    } else if(offset <= WORK_RAM_BANK1_END) {
        // 4KB Work RAM (WRAM) bank 1~N
        return mem->read_ram(offset);
    } else if(offset <= ECHO_RAM_END) {
        // Mirror of C000~DDFF (ECHO RAM)
        return mem->read_ram(offset - ECHO_RAM_START + WORK_RAM_BANK0_START);
    } else if(offset <= OBJECT_RAM_END) {
        // Sprite attribute table (OAM)
        return mem->read_oam(offset);
    } else if(offset <= UNMAPPED_END) {
        // Not Usable
        return 0;
    } else if(offset <= IO_REGS_END) {
        // Registers
        return read_reg(offset - IO_REGS_START);
    } else if(offset <= HIGH_RAM_END) {
        // High RAM (HRAM)
        return mem->read_hram(offset);
    } else if(offset <= IE_REG_OFFSET) {
        // Interrupts Enable Register (IE)
        return read_reg(offset - IO_REGS_START);
    }

    LogError("IO_Bus") << "read OOB: " << as_hex(offset);
    return 0;
}

void IO_Bus::write(u16 offset, u8 data) {
    // TODO: do we need a bypass?
    if(dma_active) {
        if(offset >= 0xFF80 && offset < 0xFFFF) {
            return mem->write_hram(offset, data);
        } else {
            return; // TODO:
        }
    }

    if(offset <= CART_ROM_BANK0_END) {
        // 16KB ROM bank 00
        cart->write(offset, data);
        return;
    } else if(offset <= CART_ROM_BANK1_END) {
        // 16KB ROM Bank 01~NN
        cart->write(offset, data);
        return;
    } else if(offset <= VIDEO_RAM_END) {
        // 8KB ppu RAM (VRAM)
        mem->write_vram(offset, data);
        return;
    } else if(offset <= CART_RAM_END) {
        // 8KB External RAM
        cart->write(offset, data);
        return;
    } else if(offset <= WORK_RAM_BANK0_END) {
        // 4KB Work RAM (WRAM) bank 0
        mem->write_ram(offset, data);
        return;
    } else if(offset <= WORK_RAM_BANK1_END) {
        // 4KB Work RAM (WRAM) bank 1~N
        mem->write_ram(offset, data);
        return;
    } else if(offset <= ECHO_RAM_END) {
        // Mirror of C000~DDFF (ECHO RAM)
        mem->write_ram(offset - 0xE000 + 0xC000, data);
        return;
    } else if(offset <= OBJECT_RAM_END) {
        // Sprite attribute table (OAM)
        mem->write_oam(offset, data);
        return;
    } else if(offset <= UNMAPPED_END) {
        // Not Usable
        return;
    } else if(offset <= IO_REGS_END) {
        // Registers
        write_reg(offset - 0xFF00, data);
        return;
    } else if(offset <= HIGH_RAM_END) {
        // High RAM (HRAM)
        mem->write_hram(offset, data);
        return;
    } else if(offset <= 0xFFFF) {
        // Interrupts Enable Register (IE)
        write_reg(offset - 0xFF00, data);
        return;
    }

    LogError("IO_Bus") << "write OOB: " << as_hex(offset);
}

// Some Registers have special behavior (such as instantaneous sampling and on-change behavior)
// and putting this in mem would introduce cyclic dependencies, so we introduce register-IO wrapper functions to handle
// it
u8 IO_Bus::read_reg(u8 loc) {
    switch(loc) {
    case P1_REG:   return P1_DEFAULTS | joy->read();

    // Timer Registers
    case DIV_REG:  return div_cnt >> 8;

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
    case NR52_REG: return apu->read_reg(loc);
    case BCPD_REG: return ppu->read_bg_color_data();
    case OCPD_REG: return ppu->read_obj_color_data();
    }

    return mem->read_reg(loc);
}

void IO_Bus::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case P1_REG:
        if(cart->cartSupportsSGB() && (data & P1_WRITE_MASK) == 0) {
            LogInfo("IO_Bus") << "Game may be attempting a SGB Command?";
        }

        joy->write(data & P1_WRITE_MASK);
        return;
    case DIV_REG:  div_cnt = 0; return;

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
    case NR52_REG: return apu->write_reg(loc, data);

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
    case 0x3F:     return apu->write_wavram(loc, data);
    case LCDC_REG:
        // TODO: this can be moved to ppu.cpp or removed
        if(!Bit::test(data, 7) && !check_ppu_mode(MODE_VBLANK)) {
            LogError("IO_Bus") << "LCD Disable outside VBLANK";
        }
        break;
    case DMA_REG: dma_start = true; break;
    case ROMEN_REG:
        LogDebug("IO_Bus") << "Boot rom disabled";
        bootrom_mode = false;
        return;
    case HDMA5_REG:
        LogDebug("IO_Bus") << "starting " << (Bit::test(data, 7) ? "HDMA" : "GDMA");
        if(hdma_active) {
            hdma_active = false;
            if(Bit::test(data, 7)) {
                hdma_start = true;
            } else {
                data |= 0x80;
            }
        } else {
            if(Bit::test(data, 7)) {
                hdma_start = true;
            } else {
                gdma_start = true;
            }
        }
        break;
    case BCPD_REG: ppu->write_bg_color_data(data); return;
    case OCPD_REG: ppu->write_obj_color_data(data); return;
    case OPRI_REG: ppu->set_obj_priority(data); return;
    case 0x72:
    // TODO
    case 0x73:
    // TODO
    case 0x74:
        if(dev_is_GBC(device)) {
            // TODO
        }
    case 0x75:
    case 0x76:
    case 0x77: return;
    }

    mem->write_reg(loc, data);
}

void IO_Bus::dma_tick() {
    if(dma_active) {
        if(dma_tick_cnt % 4 == 0) {
            u16 src = (u16)reg(DMA) << 8 | dma_byte_cnt, dest = 0xFE00 | dma_byte_cnt;
            mem->write_oam(dest, read(src, true));
            dma_byte_cnt++;
        }
        dma_tick_cnt++;

        if(dma_byte_cnt == 0xa0) {
            dma_active = false;
        }
    }

    // delay start by 1 dma tick
    if(dma_start_active) {
        dma_start_active = false;
        dma_active       = true;
    }

    if(dma_start) {
        dma_start        = false;
        dma_start_active = true;
        dma_tick_cnt     = 0;
        dma_byte_cnt     = 0;
    }
}

void IO_Bus::gdma_tick() {
    if(gdma_active) {
        if(gdma_tick_cnt % 32 == 0) {
            gbc_dma_copy_block();
        }
        gdma_tick_cnt++;

        if(reg(HDMA5) == 0x7F) {
            gdma_active = false;
            reg(HDMA5) |= 0x80;

            gdma_tick_cnt = 0;
        }
    }

    if(gdma_start) {
        if(gdma_tick_cnt % 4 == 0) {
            gdma_start    = false;
            gdma_tick_cnt = 0;
        }
        gdma_tick_cnt++;
    }
}

void IO_Bus::hdma_tick() {
    if(hdma_active) {
        if(check_ppu_mode(MODE_HBLANK)) {
            if(hdma_can_copy) {
                hdma_can_copy = false;
                gbc_dma_copy_block();
            }
        } else {
            hdma_can_copy = true;
        }

        if(reg(HDMA5) == 0xFF) {
            hdma_active = false;
        }
    }

    if(hdma_start) {
        reg(HDMA5) &= 0x7F;
        hdma_start    = false;
        hdma_active   = true;
        hdma_can_copy = true;
    }
}

void IO_Bus::gbc_dma_copy_block() {
#define regs_to_u16(H, L) (((u16)(reg(H)) << 8) | reg(L))
#define regs_from_u16(H, L, D) \
    do { \
        decltype(D) _D = D; \
        reg(H)         = _D >> 8; \
        reg(L)         = _D & 0xFF; \
    } while(0)

    u16 src  = regs_to_u16(HDMA1, HDMA2);
    u16 dest = regs_to_u16(HDMA3, HDMA4);
    for(int i = 0; i < 0x10; i++) {
        write(dest + i, read(src + i));
    }
    regs_from_u16(HDMA1, HDMA2, src + 0x10);
    regs_from_u16(HDMA3, HDMA4, dest + 0x10);

    reg(HDMA5) = (reg(HDMA5) & 0x80) | ((reg(HDMA5) - 1) & 0x7F);

#undef regs_from_u16
#undef regs_to_u16
}
