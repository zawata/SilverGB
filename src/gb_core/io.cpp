#include "gb_core/defs.hpp"
#include "gb_core/io.hpp"
#include "gb_core/io_reg.hpp"

#include "util/bit.hpp"
#include <stdexcept>

#define MODE_HBLANK 0
#define MODE_VBLANK 1
#define MODE_OAM    2
#define MODE_VRAM   3
#define check_mode(x) ((registers.STAT & 0x3) == x)

IO_Bus::IO_Bus(Cartridge *cart, bool gbc_mode, Silver::File *bios_file = nullptr) :
snd(new Sound_Controller()),
input(new Input_Manager()),
cart(cart),
gbc_mode(gbc_mode),
bootrom_file(bios_file),
bootrom_mode(bios_file != nullptr) {
    registers = { 0 };
    if(bios_file == nullptr) {
       //C++ doesn't support direct-initialization of structs until
       // c++20 which im not going to target right now, so just init the annoying way
       registers.TIMA = 0x00;
       registers.TMA  = 0x00;
       registers.TAC  = 0x00;
       write(0xFF00 + NR10_REG, 0x80);
       write(0xFF00 + NR11_REG, 0xBF);
       write(0xFF00 + NR12_REG, 0xF3);
       write(0xFF00 + NR14_REG, 0xBF);
       write(0xFF00 + NR21_REG, 0x3F);
       write(0xFF00 + NR22_REG, 0x00);
       write(0xFF00 + NR24_REG, 0xBF);
       write(0xFF00 + NR30_REG, 0x7F);
       write(0xFF00 + NR31_REG, 0xFF);
       write(0xFF00 + NR32_REG, 0x9F);
       write(0xFF00 + NR33_REG, 0xBF);
       write(0xFF00 + NR41_REG, 0xFF);
       write(0xFF00 + NR42_REG, 0x00);
       write(0xFF00 + NR43_REG, 0x00);
       write(0xFF00 + NR44_REG, 0xBF);
       write(0xFF00 + NR50_REG, 0x77);
       write(0xFF00 + NR51_REG, 0xF3);
       write(0xFF00 + NR52_REG, 0xF1);
       registers.LCDC = 0x91;
       registers.SCY  = 0x00;
       registers.SCX  = 0x00;
       registers.LYC  = 0x00;
       registers.BGP  = 0xFC;
       registers.OBP0 = 0xFF;
       registers.OBP1 = 0xFF;
       registers.WY   = 0x00;
       registers.WX   = 0x00;
       registers.IE   = 0x00;
    }

    if(gbc_mode) {
        work_ram.resize(GBC_WORK_RAM_SIZE);
        video_ram_char.resize(GBC_VRAM_CHAR_SIZE);
    }
    else {
        work_ram.resize(DMG_WORK_RAM_SIZE);
        video_ram_char.resize(DMG_VRAM_CHAR_SIZE);
    }

    high_ram.resize(HIGH_RAM_SIZE);
    video_ram_back.resize(VRAM_BACK_SIZE);
    oam_ram.resize(OAM_RAM_SIZE);
}

IO_Bus::~IO_Bus() {
    delete snd;
    delete input;
}

u8 IO_Bus::read(u16 offset, bool bypass) {
    if(dma_active && !bypass) {
        if(offset >= 0xFF80 && offset < 0xFFFF) {
            return read_hram(offset);
        } else {
            return 0; //TODO:
        }
    }

    if(offset <= 0xFF) {
        if(bootrom_mode) {
            return bootrom_file->getByte(offset);
        } else {
            return cart->read_rom(offset);
        }
    }
    else if(offset <= 0x3FFF) {
    // 16KB ROM bank 00
        return cart->read_rom(offset);
    }
    else if(offset <= 0x7FFF) {
    // 16KB ROM Bank 01~NN
        return cart->read_rom(offset); //cart will handle banking
    }
    else if(offset <= 0x9FFF) {
    // 8KB Video RAM (VRAM)
        return read_vram(offset);
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        return cart->read_ram(offset);
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
        return read_ram(offset);
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
        return read_ram(offset);
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        return read_ram(offset - 0xE000 + 0xC000);
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
        return read_oam(offset);
    }
    else if(offset <= 0xFEFF) {
    // Not Usable
        return 0;
    }
    else if(offset <= 0xFF7F) {
    // I/O Registers
        return read_reg(offset-0xFF00);
    }
    else if(offset <= 0xFFFE) {
    // High RAM (HRAM)
        return read_hram(offset);
    }
    else if(offset <= 0xFFFF) {
    // Interrupts Enable Register (IE)
        return read_reg(offset-0xFF00);
    }

    std::cerr << "IO Overread" << std::endl;
    return 0;
}

void IO_Bus::write(u16 offset, u8 data) {
    //TODO: do we need a bypass?
    if(dma_active) {
        if(offset >= 0xFF80 && offset < 0xFFFF) {
            return write_hram(offset, data);
        } else {
            return; //TODO:
        }
    }

    if(offset <= 0x3FFF) {
    // 16KB ROM bank 00
        cart->write_rom(offset, data);
        return;
    }
    else if(offset <= 0x7FFF) {
    // 16KB ROM Bank 01~NN
        cart->write_rom(offset, data);
        return;
    }
    else if(offset <= 0x9FFF) {
    // 8KB Video RAM (VRAM)
        write_vram(offset, data);
        return;
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        cart->write_ram(offset, data);
        return;
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
        write_ram(offset, data);
        return;
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
        write_ram(offset, data);
        return;
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        write_ram(offset - 0xE000 + 0xC000, data);
        return;
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
        write_oam(offset, offset);
        return;
    }
    else if(offset <= 0xFEFF) {
    // Not Usable
        return;
    }
    else if(offset <= 0xFF7F) {
    // I/O Registers
        write_reg(offset - 0xFF00, data);
        return;
    }
    else if(offset <= 0xFFFE) {
    // High RAM (HRAM)
        write_hram(offset, data);
        return;
    }
    else if(offset <= 0xFFFF) {
    // Interrupts Enable Register (IE)
        write_reg(offset - 0xFF00, data);
        return;
    }

    std::cerr << "IO Overwrite: " << as_hex(offset) << std::endl;
}

u8  IO_Bus::read_reg(u8 loc) {
    switch(loc) {
    case P1_REG   :
        return P1_DEFAULTS | input->read_inputs(registers.P1);

    //Serial Registers
    case SB_REG   :
        return registers.SB & SB_READ_MASK;
    case SC_REG   :
        return SC_DEFAULTS | (registers.SC & SC_READ_MASK);

    //Timer Registers
    case DIV_REG:
        return div_cnt >> 8;
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
        return snd->read_reg(loc);

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
/*
    case KEY1_REG :
*/
    case ROMEN_REG:
        return 0;
/*
    case HDMA1_REG:
    case HDMA2_REG:
    case HDMA3_REG:
    case HDMA4_REG:
    case HDMA5_REG:
    case RP_REG:
    case BCPS_REG:
    case BCPD_REG:
    case OCPS_REG:
    case OCPD_REG:
*/
    case SVBK_REG:
        return registers.SVBK & SVBK_READ_MASK;
    case IE_REG:
        return registers.IE & IE_READ_MASK;
    default:
        std::cerr << "REG err at " << as_hex(loc) << std::endl;
        return 0;
    }
}

void IO_Bus::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case P1_REG   :
        registers.P1 = P1_DEFAULTS | (data & P1_WRITE_MASK);
        return;
    case SB_REG   :
        registers.SB = data & SB_WRITE_MASK;
        return;
    case SC_REG   :
        registers.SC = SC_DEFAULTS | (data & SC_WRITE_MASK);
        return;
    case DIV_REG  :
        div_cnt = 0;
        return;
    case TIMA_REG :
        registers.TIMA = data & TIMA_WRITE_MASK;
        return;
    case TMA_REG  :
        registers.TMA = data & TMA_WRITE_MASK;
        return;
    case TAC_REG  :
        registers.TAC = TAC_DEFAULTS | (data & TAC_WRITE_MASK);
        return;
    case IF_REG   :
        registers.IF = IF_DEFAULTS | (data & IF_WRITE_MASK);
        return;

    //Sound Registers
    case NR10_REG: case NR11_REG: case NR12_REG: case NR13_REG: case NR14_REG:
                   case NR21_REG: case NR22_REG: case NR23_REG: case NR24_REG:
    case NR30_REG: case NR31_REG: case NR32_REG: case NR33_REG: case NR34_REG:
                   case NR41_REG: case NR42_REG: case NR43_REG: case NR44_REG:
    case NR50_REG: case NR51_REG: case NR52_REG:
        return snd->write_reg(loc, data);

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
        return snd->write_wavram(loc, data);
    case LCDC_REG :
        if(!Bit.test(data, 7) && !check_mode(MODE_VBLANK))
            std::cerr << "LCD Disable outside VBLANK" << std::endl;
        registers.LCDC = data & LCDC_WRITE_MASK;
        break;
    case STAT_REG:
        registers.STAT = STAT_DEFAULTS | (data & STAT_WRITE_MASK);
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
        dma_start = true;
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
    // case KEY1_REG :
    case ROMEN_REG:
        std::cout << "Boot rom disabled" << std::endl;
        bootrom_mode = false;
        return;
    // case HDMA1_REG:
    // case HDMA2_REG:
    // case HDMA3_REG:
    // case HDMA4_REG:
    // case HDMA5_REG:
    // case RP_REG   :
    // case BCPS_REG :
    // case BCPD_REG :
    // case OCPS_REG :
    // case OCPD_REG :

    case SVBK_REG:
        registers.SVBK = data & SVBK_WRITE_MASK;
        //value 1-7 is bank 1-7
        //value 0   is bank 1
        if(registers.SVBK)
            bank_offset = registers.SVBK * WORK_RAM_BANK_SIZE;
        else
            bank_offset = WORK_RAM_BANK_SIZE;
    case IE_REG   :
        registers.IE = data & IE_WRITE_MASK;
        return;
    default:
        std::cerr << "REG err at " << as_hex(loc) << std::endl;
        return;
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
u8 IO_Bus::read_vram(u16 offset, bool bypass) {
    if(check_mode(MODE_VRAM) && !bypass) return 0xFF;

    if(offset >= 0x8000 && offset <= 0x9FFF) {
        offset -= 0x8000;

        if(offset < VRAM_BANK_SIZE) {
            if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
                return video_ram_char[VRAM_BANK_SIZE + offset];
            }
            else {
                return video_ram_char[offset];
            }
        }
        else {
            return video_ram_back[offset-VRAM_BANK_SIZE];
        }
    }
    else {
        std::cerr << "vram read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void IO_Bus::write_vram(u16 offset, u8 data) {
    if(offset >= 0x8000 && offset <= 0x9FFF) {
        offset -= 0x8000;

        if(offset < VRAM_BANK_SIZE) {
            if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
                video_ram_char[VRAM_BANK_SIZE + offset] = data;
            }
            else {
                video_ram_char[offset] = data;
            }
        }
        else {
            video_ram_back[offset-VRAM_BANK_SIZE] = data;
        }
    }
    else {
        std::cerr << "vram write OOB: " << as_hex(offset) << std::endl;
    }
}

u8 IO_Bus::read_oam(u16 offset, bool bypass) {
    if((check_mode(MODE_OAM) || check_mode(MODE_VRAM)) && !bypass) return 0xFF;

    if(offset >= 0xFE00 && offset <= 0xFE9F) {
        offset -= 0xFE00;

        return oam_ram[offset];
    }
    else {
        std::cerr << "oam read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void IO_Bus::write_oam(u16 offset, u8 data) {
    if(offset >= 0xFE00 && offset <= 0xFE9F) {
        offset -= 0xFE00;

        oam_ram[offset] = data;
    }
    else {
        std::cerr << "oam write OOB: " << as_hex(offset) << std::endl;
    }
}

u8 IO_Bus::read_ram(u16 offset) {
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
        std::cerr << "ram read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void IO_Bus::write_ram(u16 offset, u8 data) {
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
        std::cerr << "ram write OOB: " << as_hex(offset) << std::endl;
    }
}

u8 IO_Bus::read_hram(u16 offset) {
    if(offset >= 0xFF80 && offset < 0xFFFF) {
        offset -= 0xFF80;

        return high_ram[offset];
    }
    else {
        std::cerr << "hram read OOB: " << as_hex(offset) << std::endl;
        return 0;
    }
}

void IO_Bus::write_hram(u16 offset, u8 data) {
    if(offset >= 0xFF80 && offset < 0xFFFF) {
        offset -= 0xFF80;

        high_ram[offset] = data;
    }
    else {
        std::cerr << "hram write OOB: " << as_hex(offset) << std::endl;
    }
}

/**
 * Interface Routines
 */

//Called for any interrupt
void IO_Bus::request_interrupt(IO_Bus::Interrupt i) {
    registers.IF |= i;
}

//called by Video_Controller
void IO_Bus::dma_tick() {
    if(dma_start) {
        dma_start = false;
        dma_start_active = true;
        dma_tick_cnt = 0;
        dma_byte_cnt = 0;
    }

    if(dma_active) {
        if(dma_tick_cnt % 4 == 0) {
            u16 src  = (u16)registers.DMA << 8 | dma_byte_cnt,
                dest = 0xFE00 | dma_byte_cnt;
            write_oam(dest, read(src, true));
            dma_byte_cnt++;
        }
        dma_tick_cnt++;
    }

    if(dma_byte_cnt == 0xa0 && dma_active) {
        dma_active = false;
    }

    if(dma_start_active) {
        if(dma_tick_cnt % 4 == 0) {
            dma_tick_cnt++;
        } else {
            dma_tick_cnt = 0;
            dma_start_active = false;
            dma_active = true;
        }
    }
}

/**
 * CPU Routines
 */

// increment DIV
u16 IO_Bus::cpu_inc_DIV() {
    registers.DIV = (div_cnt + 1) & 0xFF00;
    return ++div_cnt;
}
u16 IO_Bus::cpu_get_DIV() { return div_cnt; }

void IO_Bus::cpu_inc_TIMA() {
    registers.TIMA++;
    if(registers.TIMA == 0) {
        request_interrupt(TIMER_INT);
        registers.TIMA = registers.TMA;
    }
}

u16 IO_Bus::cpu_get_TAC_cs() {
//return the Timer Control Speed if enabled
    if(registers.TAC & 4) {
        switch(registers.TAC & 3) {
        case 0: return 1024;
        case 1: return 16;
        case 2: return 64;
        case 3: return 256;
        }
    }

    return 0; // the TIMA register won't be incremented
}

IO_Bus::Interrupt IO_Bus::cpu_check_interrupts() {
    //return Interrupts that are both set and requested.
    //The CPU will check the IME flag
    return (Interrupt)(registers.IF & registers.IE);
}

void IO_Bus::cpu_unset_interrupt(IO_Bus::Interrupt i) {
    //turn off the specific interrupt
    registers.IF &= ~(i);
}