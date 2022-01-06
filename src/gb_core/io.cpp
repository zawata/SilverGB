// #include <stdexcept>

#include <nowide/iostream.hpp>

#include "gb_core/defs.hpp"
#include "gb_core/io.hpp"
#include "gb_core/mem.hpp"
#include "gb_core/io_reg.hpp"

#include "util/bit.hpp"
#include <util/util.hpp>

#define MODE_HBLANK 0
#define MODE_VBLANK 1
#define MODE_OAM    2
#define MODE_VRAM   3
#define check_mode(x) ((mem->registers.STAT & 0x3) == x)

IO_Bus::IO_Bus(Memory *mem, APU *apu, PPU *ppu, Joypad *joy, Cartridge *cart, bool gbc_mode, Silver::File *bios_file = nullptr) :
mem(mem),
apu(apu),
ppu(ppu),
joy(joy),
cart(cart),
gbc_mode(gbc_mode),
bootrom_mode(bios_file != nullptr) {
    if(bios_file == nullptr) {
       //C++ doesn't support direct-initialization of structs until
       // c++20 which im not going to target right now, so just init the annoying way

       //FIXME: this doesn't bypass on-write effects!
       write_reg(TIMA_REG, 0x00);
       write_reg(TMA_REG,  0x00);
       write_reg(TAC_REG,  0x00);
       write_reg(NR10_REG, 0x80);
       write_reg(NR11_REG, 0xBF);
       write_reg(NR12_REG, 0xF3);
       write_reg(NR14_REG, 0xBF);
       write_reg(NR21_REG, 0x3F);
       write_reg(NR22_REG, 0x00);
       write_reg(NR24_REG, 0xBF);
       write_reg(NR30_REG, 0x7F);
       write_reg(NR31_REG, 0xFF);
       write_reg(NR32_REG, 0x9F);
       write_reg(NR33_REG, 0xBF);
       write_reg(NR41_REG, 0xFF);
       write_reg(NR42_REG, 0x00);
       write_reg(NR43_REG, 0x00);
       write_reg(NR44_REG, 0xBF);
       write_reg(NR50_REG, 0x77);
       write_reg(NR51_REG, 0xF3);
       write_reg(NR52_REG, 0xF1);
       write_reg(LCDC_REG, 0x91);
       write_reg(SCY_REG,  0x00);
       write_reg(SCX_REG,  0x00);
       write_reg(LYC_REG,  0x00);
       write_reg(BGP_REG,  0xFC);
       write_reg(OBP0_REG, 0xFF);
       write_reg(OBP1_REG, 0xFF);
       write_reg(WY_REG,   0x00);
       write_reg(WX_REG,   0x00);
       write_reg(IE_REG,   0x00);
    } else {
        bios_file->toVector(bootrom_buffer);
    }

}

IO_Bus::~IO_Bus() {}

u8 IO_Bus::read(u16 offset, bool bypass) {
    if(dma_active && !bypass) {
        if(offset >= 0xFF80 && offset < 0xFFFF) {
            return mem->read_hram(offset);
        } else {
            return 0; //TODO:
        }
    }

    if(offset <= 0xFF) {
        if(bootrom_mode) {
            return bootrom_buffer[offset];
        } else {
            return cart->read(offset);
        }
    }
    else if(offset <= 0x3FFF) {
    // 16KB ROM bank 00
        return cart->read(offset);
    }
    else if(offset <= 0x7FFF) {
    // 16KB ROM Bank 01~NN
        return cart->read(offset); //cart will handle banking
    }
    else if(offset <= 0x9FFF) {
    // 8KB ppu RAM (VRAM)
        return mem->read_vram(offset);
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        return cart->read(offset);
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
        return mem->read_ram(offset);
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
        return mem->read_ram(offset);
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        return mem->read_ram(offset - 0xE000 + 0xC000);
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
        return mem->read_oam(offset);
    }
    else if(offset <= 0xFEFF) {
    // Not Usable
        return 0;
    }
    else if(offset <= 0xFF7F) {
    // Registers
        return read_reg(offset-0xFF00);
    }
    else if(offset <= 0xFFFE) {
    // High RAM (HRAM)
        return mem->read_hram(offset);
    }
    else if(offset <= 0xFFFF) {
    // Interrupts Enable Register (IE)
        return read_reg(offset-0xFF00);
    }

    nowide::cerr << "IO Overread" << std::endl;
    return 0;
}

void IO_Bus::write(u16 offset, u8 data) {
    //TODO: do we need a bypass?
    if(dma_active) {
        if(offset >= 0xFF80 && offset < 0xFFFF) {
            return mem->write_hram(offset, data);
        } else {
            return; //TODO:
        }
    }

    if(offset <= 0x3FFF) {
    // 16KB ROM bank 00
        cart->write(offset, data);
        return;
    }
    else if(offset <= 0x7FFF) {
    // 16KB ROM Bank 01~NN
        cart->write(offset, data);
        return;
    }
    else if(offset <= 0x9FFF) {
    // 8KB ppu RAM (VRAM)
        mem->write_vram(offset, data);
        return;
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        cart->write(offset, data);
        return;
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
        mem->write_ram(offset, data);
        return;
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
        mem->write_ram(offset, data);
        return;
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        mem->write_ram(offset - 0xE000 + 0xC000, data);
        return;
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
        mem->write_oam(offset, data);
        return;
    }
    else if(offset <= 0xFEFF) {
    // Not Usable
        return;
    }
    else if(offset <= 0xFF7F) {
    // Registers
        write_reg(offset - 0xFF00, data);
        return;
    }
    else if(offset <= 0xFFFE) {
    // High RAM (HRAM)
        mem->write_hram(offset, data);
        return;
    }
    else if(offset <= 0xFFFF) {
    // Interrupts Enable Register (IE)
        write_reg(offset - 0xFF00, data);
        return;
    }

    nowide::cerr << "IO Overwrite: " << as_hex(offset) << std::endl;
}

// Some REgisters have special behavior (such as instantaneous sampling and on-change behavior)
// and putting this in mem would introduce cyclic dependencies, so we introduce register-IO wrapper functions to handle it
u8 IO_Bus::read_reg(u8 loc) {
    switch(loc) {
    case P1_REG:
        return P1_DEFAULTS | joy->read();

    //Timer Registers
    case DIV_REG:
        return div_cnt >> 8;

    //Sound Registers
    case NR10_REG: case NR11_REG: case NR12_REG: case NR13_REG: case NR14_REG:
                   case NR21_REG: case NR22_REG: case NR23_REG: case NR24_REG:
    case NR30_REG: case NR31_REG: case NR32_REG: case NR33_REG: case NR34_REG:
                   case NR41_REG: case NR42_REG: case NR43_REG: case NR44_REG:
    case NR50_REG: case NR51_REG: case NR52_REG:
        return apu->read_reg(loc);
    }

    return mem->read_reg(loc);
}

void IO_Bus::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case P1_REG:
        joy->write(data & P1_WRITE_MASK);
        return;
    case DIV_REG:
        div_cnt = 0;
        return;

    //Sound Registers
    case NR10_REG: case NR11_REG: case NR12_REG: case NR13_REG: case NR14_REG:
                   case NR21_REG: case NR22_REG: case NR23_REG: case NR24_REG:
    case NR30_REG: case NR31_REG: case NR32_REG: case NR33_REG: case NR34_REG:
                   case NR41_REG: case NR42_REG: case NR43_REG: case NR44_REG:
    case NR50_REG: case NR51_REG: case NR52_REG:
        return apu->write_reg(loc, data);

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
        return apu->write_wavram(loc, data);
    case LCDC_REG:
        //TODO: this can be moved to ppu.cpp or removed
        if(!Bit::test(data, 7) && !check_mode(MODE_VBLANK))
            nowide::cerr << "LCD Disable outside VBLANK" << std::endl;
        break;
    case DMA_REG:
        dma_start = true;
        break;
    case ROMEN_REG:
        nowide::cout << "Boot rom disabled" << std::endl;
        bootrom_mode = false;
        return;
    case SVBK_REG:
        mem->registers.SVBK = data & SVBK_WRITE_MASK;
        //value 1-7 is bank 1-7
        //value 0   is bank 1
        if(mem->registers.SVBK == 0)
            bank_offset = mem->registers.SVBK * WORK_RAM_BANK_SIZE;
        else
            bank_offset = WORK_RAM_BANK_SIZE;
        break;
    }

    mem->write_reg(loc, data);
}

void IO_Bus::dma_tick() {
    if(dma_active) {
        if(dma_tick_cnt % 4 == 0) {
            u16 src  = (u16)mem->registers.DMA << 8 | dma_byte_cnt,
                dest = 0xFE00 | dma_byte_cnt;
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
        dma_active = true;
    }

    if(dma_start) {
        dma_start = false;
        dma_start_active = true;
        dma_tick_cnt = 0;
        dma_byte_cnt = 0;
    }
}
