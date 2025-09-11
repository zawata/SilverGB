#pragma once

#include "util/flags.hpp"
#include "util/types/primitives.hpp"

#define GB_BOOTROM_SZ   0x0100
#define GBC_BOOTROM_SZ  0x0900

#define TICKS_PER_FRAME 70224

// inclusive
enum gb_mem_map_t : u16 {
    GB_BOOTROM_START     = 0,
    GB_BOOTROM_END       = 0xFF,
    GBC_BOOTROM_START    = 0x200,
    GBC_BOOTROM_END      = 0x8FF,

    CART_ROM_BANK0_START = 0,
    CART_ROM_BANK0_END   = 0x3FFF,
    CART_ROM_BANK1_START = 0x4000,
    CART_ROM_BANK1_END   = 0x7FFF,

    VIDEO_RAM_START      = 0x8000,
    VIDEO_RAM_END        = 0x9FFF,

    CART_RAM_START       = 0xA000,
    CART_RAM_END         = 0xBFFF,

    WORK_RAM_BANK0_START = 0xC000,
    WORK_RAM_BANK0_END   = 0xCFFF,
    WORK_RAM_BANK1_START = 0xD000,
    WORK_RAM_BANK1_END   = 0xDFFF,

    ECHO_RAM_START       = 0xE000,
    ECHO_RAM_END         = 0xFDFF,

    OBJECT_RAM_START     = 0xFE00,
    OBJECT_RAM_END       = 0xFE9F,

    UNMAPPED_START       = 0xFEA0,
    UNMAPPED_END         = 0xFEFF,

    IO_REGS_START        = 0xFF00,
    IO_REGS_END          = 0xFF7F,

    HIGH_RAM_START       = 0xFF80,
    HIGH_RAM_END         = 0xFFFE,

    IE_REG_OFFSET        = 0xFFFF
};

enum gb_device_t {
    // Gameboy
    device_GB_cpu_dmg0,
    device_GB_cpu_dmg,
    device_GB = device_GB_cpu_dmg,

    // Gameboy Pocket
    device_MGB_cpu_mgb,
    device_MGB = device_MGB_cpu_mgb,

    // Super Gameboy
    device_SGB_cpu_sgb,
    device_SGB = device_SGB_cpu_sgb,

    // Super Gameboy 2
    device_SGB2_cpu_sgb2,
    device_SGB2 = device_SGB2_cpu_sgb2,

    // Gameboy Color
    device_GBC_cpu_cgb0,
    device_GBC_cpu_cgb,
    device_GBC_cpu_cgb_agb,
    device_GBC = device_GBC_cpu_cgb,
};

constexpr const char *cpu_names[] = {"dmg0", "dmg", "mgb", "gb", "sgb2", "cgb0", "cgb", "cgb_agb"};

#define dev_is_GB(x)  ((x) == device_GB_cpu_dmg0 || (x) == device_GB_cpu_dmg || (x) == device_MGB_cpu_mgb)

#define dev_is_SGB(x) ((x) == device_SGB_cpu_sgb || (x) == device_SGB2_cpu_sgb2)

#define dev_is_GBC(x) ((x) == device_GBC_cpu_cgb0 || (x) == device_GBC_cpu_cgb || (x) == device_GBC_cpu_cgb_agb)

enum bootrom_crc_ { DMG_BOOTROM_CRC = 0x59c8598e };

extern u8       get_default_idx(gb_device_t device);

extern const u8 default_reg_values[55][4];
extern const u8 oam_ram_CGB_initial_state[];
extern const u8 ppu_ram_CGB_initial_state[];
extern const u8 high_ram_CGB_initial_state[];
extern const u8 work_ram_CGB_initial_state[];

// Every register has defined its:
//  - Location from the IO Offset
//  - Bits that can be written in the form of a mask
//  - Bits that can be read in the form of a mask

// Ports Register
#define P1_REG           0x00
#define P1_WRITE_MASK    0x30
#define P1_READ_MASK     P1_WRITE_MASK
#define P1_DEFAULTS      0xC0

// Serial Data
#define SB_REG           0x01
#define SB_WRITE_MASK    0xFF
#define SB_READ_MASK     SB_WRITE_MASK

// Serial Clock
#define SC_REG           0x02
#define SC_WRITE_MASK    0x81 // TODO: CGB Clock Speed
#define SC_READ_MASK     SC_WRITE_MASK
#define SC_DEFAULTS      ~SC_WRITE_MASK

// Divider
#define DIV_REG          0x04
#define DIV_WRITE_MASK   0x00
#define DIV_READ_MASK    DIV_WRITE_MASK

// Timer Counter
#define TIMA_REG         0x05
#define TIMA_WRITE_MASK  0xFF
#define TIMA_READ_MASK   TIMA_WRITE_MASK

// Timer Modulo
#define TMA_REG          0x06
#define TMA_WRITE_MASK   0xFF
#define TMA_READ_MASK    TMA_WRITE_MASK

// Timer Controller
#define TAC_REG          0x07
#define TAC_WRITE_MASK   0x07
#define TAC_READ_MASK    TAC_WRITE_MASK
#define TAC_DEFAULTS     ~TAC_WRITE_MASK

// Interrupt Flags
#define IF_REG           0x0F
#define IF_WRITE_MASK    0x1F
#define IF_READ_MASK     IF_WRITE_MASK
#define IF_DEFAULTS      ~IF_WRITE_MASK

// Channel 1 Sweep register
#define NR10_REG         0x10
#define NR10_WRITE_MASK  0x7F
#define NR10_READ_MASK   NR10_WRITE_MASK

// Channel 1 Sound length/Wave pattern duty
#define NR11_REG         0x11
#define NR11_WRITE_MASK  0xFF
#define NR11_READ_MASK   0xC0

// Channel 1 Volume Envelope (R/W)
#define NR12_REG         0x12
#define NR12_WRITE_MASK  0xFF
#define NR12_READ_MASK   NR12_WRITE_MASK

// Channel 1 Frequency lo (Write Only)
#define NR13_REG         0x13
#define NR13_WRITE_MASK  0xFF
#define NR13_READ_MASK   0x00

// Channel 1 Frequency hi (R/W)
#define NR14_REG         0x14
#define NR14_WRITE_MASK  0xC7
#define NR14_READ_MASK   0x40

// Channel 2 Sweep register (Non-existant)
#define NR20_REG         0x15
#define NR20_WRITE_MASK  0x00
#define NR20_READ_MASK   0x00
#define NR20_DEFAULTS    0xFF

// Channel 2 Sound Length/Wave Pattern Duty (R/W)
#define NR21_REG         0x16
#define NR21_WRITE_MASK  0xFF
#define NR21_READ_MASK   0xC0

// Channel 2 Volume Envelope (R/W)
#define NR22_REG         0x17
#define NR22_WRITE_MASK  0xFF
#define NR22_READ_MASK   NR21_WRITE_MASK

// Channel 2 Frequency lo data (W)
#define NR23_REG         0x18
#define NR23_WRITE_MASK  0xFF
#define NR23_READ_MASK   0x00

// Channel 2 Frequency hi data (R/W)
#define NR24_REG         0x19
#define NR24_WRITE_MASK  0xC7
#define NR24_READ_MASK   0x40

// Channel 3 Sound on/off (R/W)
#define NR30_REG         0x1A
#define NR30_WRITE_MASK  0x80
#define NR30_READ_MASK   NR30_WRITE_MASK

// Channel 3 Sound Length
#define NR31_REG         0x1B
#define NR31_WRITE_MASK  0xFF
#define NR31_READ_MASK   NR31_WRITE_MASK

// Channel 3 Select output level (R/W)
#define NR32_REG         0x1C
#define NR32_WRITE_MASK  0x60
#define NR32_READ_MASK   NR32_WRITE_MASK

// Channel 3 Frequency's lower data (W)
#define NR33_REG         0x1D
#define NR33_WRITE_MASK  0xFF
#define NR33_READ_MASK   0x00

// Channel 3 Frequency's higher data (R/W)
#define NR34_REG         0x1E
#define NR34_WRITE_MASK  0xC7
#define NR34_READ_MASK   0x40

// Channel 4 Sweep register (Non-existant)
#define NR40_REG         0x1F
#define NR40_WRITE_MASK  0x00
#define NR40_READ_MASK   0x00
#define NR40_DEFAULTS    0xFF

// Channel 4 Sound Length (R/W)
#define NR41_REG         0x20
#define NR41_WRITE_MASK  0x3F
#define NR41_READ_MASK   NR41_WRITE_MASK

// Channel 4 Volume Envelope (R/W)
#define NR42_REG         0x21
#define NR42_WRITE_MASK  0xFF
#define NR42_READ_MASK   NR42_WRITE_MASK

// Channel 4 Polynomial Counter (R/W)
#define NR43_REG         0x22
#define NR43_WRITE_MASK  0xFF
#define NR43_READ_MASK   NR43_WRITE_MASK

// Channel 4 Counter/consecutive; Inital (R/W)
#define NR44_REG         0x23
#define NR44_WRITE_MASK  0xC0
#define NR44_READ_MASK   0x40

// Channel control / ON-OFF / Volume (R/W)
#define NR50_REG         0x24
#define NR50_WRITE_MASK  0xFF
#define NR50_READ_MASK   NR50_WRITE_MASK

// Selection of Sound output terminal (R/W)
#define NR51_REG         0x25
#define NR51_WRITE_MASK  0xFF
#define NR51_READ_MASK   NR51_WRITE_MASK

// Sound on/off
#define NR52_REG         0x26
#define NR52_WRITE_MASK  0x80
#define NR52_READ_MASK   0xFF

// LCD Control
#define LCDC_REG         0x40
#define LCDC_WRITE_MASK  0xFF
#define LCDC_READ_MASK   LCDC_WRITE_MASK

// LCDC Status
#define STAT_REG         0x41
#define STAT_WRITE_MASK  0x78
#define STAT_READ_MASK   0x7F
#define STAT_DEFAULTS    ~STAT_READ_MASK

// Scroll Y
#define SCY_REG          0x42
#define SCY_WRITE_MASK   0xFF
#define SCY_READ_MASK    LCDC_WRITE_MASK

// Scroll X
#define SCX_REG          0x43
#define SCX_WRITE_MASK   0xFF
#define SCX_READ_MASK    LCDC_WRITE_MASK

// LCDC Y-Coordinate
#define LY_REG           0x44
#define LY_WRITE_MASK    0x00
#define LY_READ_MASK     0xFF

// LY Compare
#define LYC_REG          0x45
#define LYC_WRITE_MASK   0xFF
#define LYC_READ_MASK    LYC_WRITE_MASK

// DMA Transfer and Start Address
#define DMA_REG          0x46
#define DMA_WRITE_MASK   0xFF
#define DMA_READ_MASK    DMA_WRITE_MASK

// BG Palette Data
#define BGP_REG          0x47
#define BGP_WRITE_MASK   0xFF
#define BGP_READ_MASK    BGP_WRITE_MASK

// Object Palette 0 Data
#define OBP0_REG         0x48
#define OBP0_WRITE_MASK  0xFF
#define OBP0_READ_MASK   OBP0_WRITE_MASK

// Object Palette 1 Data
#define OBP1_REG         0x49
#define OBP1_WRITE_MASK  0xFF
#define OBP1_READ_MASK   OBP1_WRITE_MASK

// Window Y
#define WY_REG           0x4A
#define WY_WRITE_MASK    0xFF
#define WY_READ_MASK     WY_WRITE_MASK

// Window X
#define WX_REG           0x4B
#define WX_WRITE_MASK    0xFF
#define WX_READ_MASK     WX_WRITE_MASK

// GBC: Compat Mode
#define KEY0_REG         0x4C
#define KEY0_WRITE_MASK  0x0C
#define KEY0_READ_MASK   KEY0_WRITE_MASK

// GBC: Prepare Speed Switch
#define KEY1_REG         0x4D
#define KEY1_WRITE_MASK  0x01
#define KEY1_READ_MASK   0x81

// GBC: VRAM Bank Switcher
#define VBK_REG          0x4F
#define VBK_WRITE_MASK   0x01
#define VBK_READ_MASK    VBK_WRITE_MASK
#define VBK_DEFAULTS     0xFE

// hidden: Bootstrap Enable
#define ROMEN_REG        0x50
#define ROMEN_WRITE_MASK 0x00
#define ROMEN_READ_MASK  ROMEN_WRITE_MASK

// GBC: HDMA Source High
#define HDMA1_REG        0x51
#define HDMA1_WRITE_MASK 0xFF
#define HDMA1_READ_MASK  HDMA1_WRITE_MASK
#define HDMA1_DEFAULTS   0xFF

// GBC: HDMA Source Low
#define HDMA2_REG        0x52
#define HDMA2_WRITE_MASK 0xF0
#define HDMA2_READ_MASK  HDMA2_WRITE_MASK
#define HDMA2_DEFAULTS   0xFF

// GBC: HDMA Destination High
#define HDMA3_REG        0x53
#define HDMA3_WRITE_MASK 0xFF
#define HDMA3_READ_MASK  HDMA3_WRITE_MASK
#define HDMA3_DEFAULTS   0xFF

// GBC: HDMA Destination Low
#define HDMA4_REG        0x54
#define HDMA4_WRITE_MASK 0xF0
#define HDMA4_READ_MASK  HDMA4_WRITE_MASK
#define HDMA4_DEFAULTS   0xFF

// GBC: HDMA Control
#define HDMA5_REG        0x55
#define HDMA5_WRITE_MASK 0xFF
#define HDMA5_READ_MASK  0xFF

// GBC: Infrared Communications Port
#define RP_REG           0x56
#define RP_WRITE_MASK    0xC1
#define RP_READ_MASK     0xFF

// GBC: Background Color Palette Specification
#define BCPS_REG         0x68
#define BCPS_WRITE_MASK  0xBF
#define BCPS_READ_MASK   BCPS_WRITE_MASK

// GBC: Background Color Palette Data
#define BCPD_REG         0x69
#define BCPD_WRITE_MASK  0x3FFF
#define BCPD_READ_MASK   BCPD_WRITE_MASK

// GBC: Object Color Palette Specification
#define OCPS_REG         0x6A
#define OCPS_WRITE_MASK  0xBF
#define OCPS_READ_MASK   OCPS_WRITE_MASK

// GBC: Object Color Palette Data
#define OCPD_REG         0x6B
#define OCPD_WRITE_MASK  0xFF
#define OCPD_READ_MASK   OCPD_WRITE_MASK

// GBC: Object Priority Mode
#define OPRI_REG         0x6C
#define OPRI_WRITE_MASK  0x01
#define OPRI_READ_MASK   OPRI_WRITE_MASK

// WRAM Bank Select
#define SVBK_REG         0x70
#define SVBK_WRITE_MASK  0x07
#define SVBK_READ_MASK   SVBK_WRITE_MASK

// Interrupt Enable
#define IE_REG           0xFF
#define IE_WRITE_MASK    0x1F
#define IE_READ_MASK     IE_WRITE_MASK
