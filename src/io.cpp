#include "io.hpp"

#include "io_reg.hpp"

IO_Bus::IO_Bus(Cartridge *cart, Configuration *config) :
cart(cart),
config(config),
vpu(new Video_Controller()),
snd(new Sound_Controller()),
input(new Input_Manager()) {
    if(config->bin_enabled) {
        boot_rom_file = File_Interface::openFile(config->bin_file);
    }
    else {
        //TODO what if we don't have the bios
    }
}

IO_Bus::~IO_Bus() {

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
        return cart->read_rom(offset);//DMG
    }
    else if(offset <= 0x9FFF) {
    // 8KB Video RAM (VRAM)
        return vpu->read_ram(offset-0x8000);
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        return cart->read_ram(offset-0xA000);
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
        return read_ram(offset-0xC000);
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
        return read_ram(offset-0xC000);//DMG
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        std::cerr << "call to ECHO RAM" << std::endl;
        return 0; //TODO: ECHO or zero?
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
        return vpu->read_oam(offset-0xFE00);
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
        vpu->write_ram(offset-0x8000, data);
    }
    else if(offset <= 0xBFFF) {
    // 8KB External RAM
        cart->write_ram(offset-0xA000, data);
    }
    else if(offset <= 0xCFFF) {
    // 4KB Work RAM (WRAM) bank 0
        write_ram(offset-0xC000, data);
    }
    else if(offset <= 0xDFFF) {
    // 4KB Work RAM (WRAM) bank 1~N
        write_ram(offset-0xC000, data);
    }
    else if(offset <= 0xFDFF) {
    // Mirror of C000~DDFF (ECHO RAM)
        std::cerr << "call to ECHO RAM" << std::endl;
        return;
    }
    else if(offset <= 0xFE9F) {
    // Sprite attribute table (OAM)
        vpu->write_oam(offset - 0xFE00, offset);
    }
    else if(offset <= 0xFEFF) {
    // Not Usable
        return;
    }
    else if(offset <= 0xFF7F) {
    // I/O Registers
        write_reg(offset - 0xFF00, data);
    }
    else if(offset <= 0xFFFE) {
    // High RAM (HRAM)

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
        return input->read_inputs(this->registers.P1);
    // case SB_REG   :
    // case SC_REG   :
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

    //TODO: this is horrendous
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
        return snd->read_reg(loc);

    case LCDC_REG:
    case STAT_REG:
    case SCY_REG:
    case SCX_REG:
    case LY_REG:
    case LYC_REG:
    case DMA_REG:
    case BGP_REG:
    case OBP0_REG:
    case OPB1_REG:
    case WY_REG:
    case WX_REG:
        return vpu->read_reg(loc);
/*
    case KEY1_REG :
    case VBK_REG  :
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
    case SVBK_REG:
*/
    case IE_REG:
        return registers.IF;
    default:
        std::cerr << "REG err at " << loc << std::endl;
    }
}

void IO_Bus::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case P1_REG   :
        this->registers.P1 = data & P1_WRITE_MASK;
        return;
    // case SB_REG   :
    //     this->registers.SB = data & SB_WRITE_MASK;
    //     return;
    // case SC_REG   :
    //     this->registers.SC = data & SC_WRITE_MASK;
    //     return;
    case DIV_REG  :
        this->registers.DIV = data & DIV_WRITE_MASK;
        return;
    case TIMA_REG :
        this->registers.TIMA = data & TIMA_WRITE_MASK;
        return;
    case TMA_REG  :
        this->registers.TMA = data & TMA_WRITE_MASK;
        return;
    case TAC_REG  :
        this->registers.TAC = data & TAC_WRITE_MASK;
        return;
    case IF_REG   :
        this->registers.IF = data & IF_WRITE_MASK;
        return;

    //TODO: this is horrendous. redo
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
        return snd->write_reg(loc, data);
#if _MSC_VER && !__INTEL_COMPILER
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x34: case 0x35: case 0x36: case 0x37:
    case 0x38: case 0x39: case 0x3A: case 0x3B:
    case 0x3C: case 0x3D: case 0x3E: case 0x3F:
#else
    //this seems to be supported on CLANG and GCC but not MSVC
    // if MSVC does support this I'll keep it
    case 0x30 ... 0x3F:
#endif
        return snd->write_wavram(loc, data);
    case LCDC_REG :
    case STAT_REG :
    case SCY_REG  :
    case SCX_REG  :
    case LY_REG   :
    case LYC_REG  :
    case DMA_REG  :
    case BGP_REG  :
    case OBP0_REG :
    case OPB1_REG :
    case WY_REG   :
    case WX_REG   :
        return vpu->write_reg(loc, data);
    // case KEY1_REG :
    // case VBK_REG  :
    case ROMEN_REG:
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
    // case SVBK_REG :
    case IE_REG   :
        this->registers.IE = data & IE_WRITE_MASK;
        return;
    default:
        std::cerr << "REG err at " << loc << std::endl;
        return;
    }
}

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
//return Interrupts that are both set and requested. The CPU will check the IME flag
    return (Interrupt)(registers.IF & registers.IE);
}

void IO_Bus::cpu_unset_interrupt(IO_Bus::Interrupt i) {
//turn off the specific interrupt
    registers.IF &= ~(i);
}