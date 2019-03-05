#include "io.hpp"

#include "io_reg.hpp"

#include "util/bit.hpp"

#include "defs.hpp"

#define check_mode(x) ((registers.STAT & 0x3) == x)

IO_Bus::IO_Bus(Cartridge *cart, Configuration *config, bool gbc_mode) :
cart(cart),
config(config),
gbc_mode(gbc_mode),
registers({ 0 }),
//vpu(new Video_Controller(gbc_mode)),
snd(new Sound_Controller()),
input(new Input_Manager()) {
    if(config->config_data.bin_enabled) {
        boot_rom_file = File_Interface::openFile(config->config_data.bin_file);
        std::cout << "Boot Rom enabled" << std::endl;

        registers = { 0 };
    }
    else {
        //TODO: cleaner exit
        std::cerr << "BIOS Emulation not supported yet..." << std::endl;
        exit(-1);
    }

    if(gbc_mode) {
        work_ram.reserve(GBC_WORK_RAM_SIZE);
        video_ram_char.reserve(GBC_VRAM_CHAR_SIZE);
    }
    else {
        work_ram.reserve(DMG_WORK_RAM_SIZE);
        video_ram_char.reserve(DMG_VRAM_CHAR_SIZE);
    }

    high_ram.reserve(HIGH_RAM_SIZE);
    video_ram_back.reserve(VRAM_BACK_SIZE);
    oam_ram.reserve(OAM_RAM_SIZE);
}

IO_Bus::~IO_Bus() {
    delete snd;
    delete input;
}

u8 IO_Bus::read(u16 offset) {
    if(offset <= 0x100) {
        if(bootrom_mode) {
            return boot_rom_file->getByte(offset);
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
        std::cout << "read to VRAM" << std::endl;
        return read_vram(offset-0x8000);
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        return cart->read_ram(offset-0xA000);
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
    std::cout << "read to work RAM" << std::endl;
        return read_ram(offset-0xC000);
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
    std::cout << "read to work RAM" << std::endl;
        return read_ram(offset-0xC000);
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        std::cerr << "read to ECHO RAM" << std::endl;
        return 0; //TODO: ECHO or zero?
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
    std::cout << "read to OAM" << std::endl;
        return read_oam(offset-0xFE00);
    }
    else if(offset <= 0xFEFF) {
    // Not Usable
        return 0;
    }
    else if(offset <= 0xFF7F) {
    // I/O Registers
        std::cout << "read to REG" << std::endl;
        return read_reg(offset-0xFF00);
    }
    else if(offset <= 0xFFFE) {
    // High RAM (HRAM)
        std::cout << "read to HRAM" << std::endl;
        return read_hram(offset-0xFF80);
    }
    else if(offset <= 0xFFFF) {
    // Interrupts Enable Register (IE)
        return read_reg(offset-0xFF00);
    }
}

void IO_Bus::write(u16 offset, u8 data) {
    if(offset <= 0x3FFF) {
    // 16KB ROM bank 00
        return; //can't write to the rom
    }
    else if(offset <= 0x7FFF) {
    // 16KB ROM Bank 01~NN
        return; //can't write to the rom
    }
    else if(offset <= 0x9FFF) {
    // 8KB Video RAM (VRAM)
        std::cout << "write to VRAM" << std::endl;
        write_vram(offset-0x8000, data);
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        cart->write_ram(offset-0xA000, data);
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
        std::cout << "write to work RAM" << std::endl;
        write_ram(offset-0xC000, data);
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
        std::cout << "write to work RAM" << std::endl;
        write_ram(offset-0xC000, data);
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        std::cerr << "write to ECHO RAM" << std::endl;
        return;
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
        std::cout << "write to OAM" << std::endl;
        write_oam(offset - 0xFE00, offset);
    }
    else if(offset <= 0xFEFF) {
    // Not Usable
        return;
    }
    else if(offset <= 0xFF7F) {
    // I/O Registers
        std::cout << "write to REG" << std::endl;
        write_reg(offset - 0xFF00, data);
    }
    else if(offset <= 0xFFFE) {
    // High RAM (HRAM)
        std::cout << "read to HRAM" << std::endl;
        write_hram(offset - 0xFF80, data);
    }
    else if(offset <= 0xFFFF) {
    // Interrupts Enable Register (IE)
        write_reg(offset - 0xFF00, data);
    }
}

u8  IO_Bus::read_reg(u8 loc) {
    switch(loc) {
    case P1_REG   :
        //TODO: I don't think this is a good idea. should probably update on CPU clock
        return input->read_inputs(registers.P1);

    //Serial Registers
    case SB_REG   :
        return registers.SB & SB_READ_MASK;
    case SC_REG   :
        return registers.SC & SB_READ_MASK;

    //Timer Registers
    case DIV_REG:
        return registers.DIV & DIV_READ_MASK;
    case TIMA_REG:
        return registers.TIMA & TIMA_READ_MASK;
    case TMA_REG:
        return registers.TMA & TMA_READ_MASK;
    case TAC_REG:
        return registers.TAC & TAC_READ_MASK;
    case IF_REG:
        return registers.IF & IF_READ_MASK;

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
        std::cerr << "REG err at " << loc << std::endl;
    }
}

void IO_Bus::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case P1_REG   :
        registers.P1 = data & P1_WRITE_MASK;
        return;
    // case SB_REG   :
    //     registers.SB = data & SB_WRITE_MASK;
    //     return;
    // case SC_REG   :
    //     registers.SC = data & SC_WRITE_MASK;
    //     return;
    case DIV_REG  :
        registers.DIV = data & DIV_WRITE_MASK;
        return;
    case TIMA_REG :
        registers.TIMA = data & TIMA_WRITE_MASK;
        return;
    case TMA_REG  :
        registers.TMA = data & TMA_WRITE_MASK;
        return;
    case TAC_REG  :
        registers.TAC = data & TAC_WRITE_MASK;
        return;
    case IF_REG   :
        registers.IF = data & IF_WRITE_MASK;
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
        if(!Bit.test(data, 7) && !check_mode(1))
            std::cerr << "LCD Disable outside VBLANK" << std::endl;
        registers.LCDC = data & STAT_WRITE_MASK;
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
        if(data & SVBK_WRITE_MASK)
            bank_offset = registers.SVBK * WORK_RAM_BANK_SIZE;
        else
            bank_offset = WORK_RAM_BANK_SIZE;
    case IE_REG   :
        registers.IE = data & IE_WRITE_MASK;
        return;
    default:
        std::cerr << "REG err at " << loc << std::endl;
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
u8 IO_Bus::read_vram(u16 loc, bool bypass) {
    if(check_mode(3) && !bypass) return 0xFF;

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

void IO_Bus::write_vram(u16 loc, u8 data) {
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

u8 IO_Bus::read_oam(u16 loc, bool bypass) {
    if((check_mode(2) || check_mode(3)) && !bypass) return 0xFF;

    return oam_ram[loc];
}

void IO_Bus::write_oam(u16 loc, u8 data) {
    oam_ram[loc] = data;
}

u8 IO_Bus::read_ram(u16 loc) {
    if(loc <= 0x0FFF) {
        return work_ram[loc];
    } else {
        if(gbc_mode) {
            return work_ram[loc + bank_offset];
        } else {
            return work_ram[loc];
        }
    }
}

void IO_Bus::write_ram(u16 loc, u8 data) {
    if(loc <= 0x0FFF) {
        work_ram[loc] = data;
    } else {
        if(gbc_mode) {
            work_ram[loc + bank_offset] = data;
        } else {
            work_ram[loc] = data;
        }
    }
}

u8 IO_Bus::read_hram(u16 loc) {
    return high_ram[loc];
}

void IO_Bus::write_hram(u16 loc, u8 data) {
    high_ram[loc] = data;
}

/**
 * Internal Routines
 */

void IO_Bus::request_interrupt(int i) {
    registers.IF |= i;
}

/**
 * CPU Routines
 */

void IO_Bus::cpu_inc_DIV() {
// increment DIV
    registers.DIV++;
}

void IO_Bus::cpu_inc_TIMA() {
//Increment TIMA. if it overflowed, request a TIMER Interrupt and set TIMA back to TMA
    if(!(++registers.TIMA)) {
        registers.IF |= TIMER_INT;
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
    } else return 0; // the TIMA register won't be incremented
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