#pragma once

//This macro will convert the register name into an array initializer of it's data
//ie. P1_DAT = __ARR(P1) = { P1_REG, P1_WRITE_MASK, P1_READ_MASK } = { 0x00, 0x30, 0x30 }
//it can be used to populate a struct for a register.
#define __ARR(X) { X##_REG, X##_WRITE_MASK, X##_READ_MASK }


//Every register has defined it's:
// - Location from the IO Offset
// - Bits it write too in the from of a mask
// - Bits it can read from in the form of a mask
// - The above data in the form of an initializer list.

// Ports Register
#define P1_REG           0x00
#define P1_WRITE_MASK    0x30
#define P1_READ_MASK     P1_WRITE_MASK
#define P1_DAT           __ARR(P1)

// Serial Data
// #define SB_REG           0x01
// #define SB_WRITE_MASK    0xFF
// #define SB_READ_MASK     SB_WRITE_MASK
// #define SB_DAT           __ARR(SB)

// Serial Clock
// #define SC_REG           0x02
// #define SC_WRITE_MASK    0x83
// #define SC_READ_MASK     SC_WRITE_MASK
// #define SC_DAT           __ARR(SC)

// Divider
#define DIV_REG          0x04
#define DIV_WRITE_MASK   0x00
#define DIV_READ_MASK    DIV_WRITE_MASK
#define DIV_DAT          __ARR(DIV)

// Timer Counter
#define TIMA_REG         0x05
#define TIMA_WRITE_MASK  0xFF
#define TIMA_READ_MASK   TIMA_WRITE_MASK
#define TIMA_DAT         __ARR(TIMA)

// Timer Modulo
#define TMA_REG          0x06
#define TMA_WRITE_MASK   0xFF
#define TMA_READ_MASK    TMA_WRITE_MASK
#define TMA_DAT          __ARR(TMA)

// Timer Controller
#define TAC_REG          0x07
#define TAC_WRITE_MASK   0x07
#define TAC_READ_MASK    TAC_WRITE_MASK
#define TAC_DAT          __ARR(TAC)

// Interrupt Flags
#define IF_REG           0x0F
#define IF_WRITE_MASK    0x1F
#define IF_READ_MASK     IF_WRITE_MASK
#define IF_DAT           __ARR(IF)

// Channel 1 Sweep register
#define NR10_REG         0x10
#define NR10_WRITE_MASK  0x7F
#define NR10_READ_MASK   NR10_WRITE_MASK
#define NR10_DAT         __ARR(NR10)

// Channel 1 Sound length/Wave pattern duty
#define NR11_REG         0x11
#define NR11_WRITE_MASK  0xFF
#define NR11_READ_MASK   0xC0
#define NR11_DAT         __ARR(NR11)

// Channel 1 Volume Envelope (R/W)
#define NR12_REG         0x12
#define NR12_WRITE_MASK  0xFF
#define NR12_READ_MASK   NR12_WRITE_MASK
#define NR12_DAT         __ARR(NR12)

// Channel 1 Frequency lo (Write Only)
#define NR13_REG         0x13
#define NR13_WRITE_MASK  0xFF
#define NR13_READ_MASK   0x00
#define NR13_DAT         __ARR(NR13)

// Channel 1 Frequency hi (R/W)
#define NR14_REG         0x14
#define NR14_WRITE_MASK  0xC7
#define NR14_READ_MASK   0x40
#define NR14_DAT         __ARR(NR14)

// Channel 2 Sound Length/Wave Pattern Duty (R/W)
#define NR21_REG         0x16
#define NR21_WRITE_MASK  0xFF
#define NR21_READ_MASK   0xC0
#define NR21_DAT         __ARR(NR21)

// Channel 2 Volume Envelope (R/W)
#define NR22_REG         0x17
#define NR22_WRITE_MASK  0xFF
#define NR22_READ_MASK   NR21_WRITE_MASK
#define NR22_DAT         __ARR(NR22)

// Channel 2 Frequency lo data (W)
#define NR23_REG         0x18
#define NR23_WRITE_MASK  0xFF
#define NR23_READ_MASK   0x00
#define NR23_DAT         __ARR(NR23)

// Channel 2 Frequency hi data (R/W)
#define NR24_REG         0x19
#define NR24_WRITE_MASK  0xC7
#define NR24_READ_MASK   0x40
#define NR24_DAT         __ARR(NR24)

// Channel 3 Sound on/off (R/W)
#define NR30_REG         0x1A
#define NR30_WRITE_MASK  0x80
#define NR30_READ_MASK   NR30_WRITE_MASK
#define NR30_DAT         __ARR(NR30)

// Channel 3 Sound Length
#define NR31_REG         0x1B
#define NR31_WRITE_MASK  0xFF
#define NR31_READ_MASK   NR31_WRITE_MASK
#define NR31_DAT         __ARR(NR31)

// Channel 3 Select output level (R/W)
#define NR32_REG         0x1C
#define NR32_WRITE_MASK  0x60
#define NR32_READ_MASK   NR32_WRITE_MASK
#define NR32_DAT         __ARR(NR32)

// Channel 3 Frequency's lower data (W)
#define NR33_REG         0x1D
#define NR33_WRITE_MASK  0xFF
#define NR33_READ_MASK   0x00
#define NR33_DAT         __ARR(NR33)

// Channel 3 Frequency's higher data (R/W)
#define NR34_REG         0x1E
#define NR34_WRITE_MASK  0xC7
#define NR34_READ_MASK   0x40
#define NR34_DAT         __ARR(NR34)

// Channel 4 Sound Length (R/W)
#define NR41_REG         0x20
#define NR41_WRITE_MASK  0x3F
#define NR41_READ_MASK   NR41_WRITE_MASK
#define NR41_DAT         __ARR(NR41)

// Channel 4 Volume Envelope (R/W)
#define NR42_REG         0x21
#define NR42_WRITE_MASK  0xFF
#define NR42_READ_MASK   NR42_WRITE_MASK
#define NR42_DAT         __ARR(NR42)

// Channel 4 Polynomial Counter (R/W)
#define NR43_REG         0x22
#define NR43_WRITE_MASK  0xFF
#define NR43_READ_MASK   NR43_WRITE_MASK
#define NR43_DAT         __ARR(NR43)

// Channel 4 Counter/consecutive; Inital (R/W)
#define NR44_REG         0x23
#define NR44_WRITE_MASK  0xC0
#define NR44_READ_MASK   0x40
#define NR44_DAT         __ARR(NR44)

// Channel control / ON-OFF / Volume (R/W)
#define NR50_REG         0x24
#define NR50_WRITE_MASK  0xFF
#define NR50_READ_MASK   NR50_WRITE_MASK
#define NR50_DAT         __ARR(NR50)

// Selection of Sound output terminal (R/W)
#define NR51_REG         0x25
#define NR51_WRITE_MASK  0xFF
#define NR51_READ_MASK   NR51_WRITE_MASK
#define NR51_DAT         __ARR(NR51)

// Sound on/off
#define NR52_REG         0x26
#define NR52_WRITE_MASK  0x80
#define NR52_READ_MASK   0xFF
#define NR52_DAT         __ARR(NR52)

// LCD Control
#define LCDC_REG         0x40
#define LCDC_WRITE_MASK  0xFF
#define LCDC_READ_MASK   LCDC_WRITE_MASK
#define LCDC_DAT         __ARR(LCDC)

// LCDC Status
#define STAT_REG         0x41
#define STAT_WRITE_MASK  0xF8
#define STAT_READ_MASK   0xFF
#define STAT_DAT         __ARR(STAT)

// Scroll Y
#define SCY_REG          0x42
#define SCY_WRITE_MASK   0xFF
#define SCY_READ_MASK    LCDC_WRITE_MASK
#define SCY_DAT          __ARR(SCY)

// Scroll X
#define SCX_REG          0x43
#define SCX_WRITE_MASK   0xFF
#define SCX_READ_MASK    LCDC_WRITE_MASK
#define SCX_DAT          __ARR(SCX)

// LCDC Y-Coordinate
#define LY_REG           0x44
#define LY_WRITE_MASK     0x00
#define LY_READ_MASK     0xFF
#define LY_DAT           __ARR(LY)

// LY Compare
#define LYC_REG          0x45
#define LYC_WRITE_MASK    0xFF
#define LYC_READ_MASK    LYC_WRITE_MASK
#define LYC_DAT          __ARR(LYC)

// DMA Transfer and Start Address
#define DMA_REG          0x46
#define DMA_WRITE_MASK    0xFF
#define DMA_READ_MASK    DMA_WRITE_MASK
#define DMA_DAT          __ARR(DMA)

// BG Palette Data
#define BGP_REG          0x47
#define BGP_WRITE_MASK   0xFF
#define BGP_READ_MASK    BGP_WRITE_MASK
#define BGP_DAT          __ARR(BGP)

// Object Palette 0 Data
#define OBP0_REG         0x48
#define OBP0_WRITE_MASK   0xFF
#define OBP0_READ_MASK   OBP0_WRITE_MASK
#define OBP0_DAT         __ARR(OBP0)

// Object Palette 1 Data
#define OBP1_REG         0x49
#define OBP1_WRITE_MASK   0xFF
#define OBP1_READ_MASK   OBP1_WRITE_MASK
#define OBP1_DAT         __ARR(OBP1)

// Window Y
#define WY_REG           0x4A
#define WY_WRITE_MASK    0xFF
#define WY_READ_MASK     WY_WRITE_MASK
#define WY_DAT           __ARR(WY)

// Window X
#define WX_REG           0x4B
#define WX_WRITE_MASK    0xFF
#define WX_READ_MASK     WX_WRITE_MASK
#define WX_DAT           __ARR(WX)

// GBC: Prepare Speed Switch
#define KEY1_REG         0x4D
#define KEY1_WRITE_MASK  0x01
#define KEY1_READ_MASK   0x81
#define KEY1_DAT         __ARR(KEY1)

// GBC: VRAM Bank Switcher
#define VBK_REG          0x4F
#define VBK_WRITE_MASK   0x01
#define VBK_READ_MASK    VBK_WRITE_MASK
#define VBK_DAT          __ARR(VBK)

// hidden: Bootstrap Enable
#define ROMEN_REG        0x50
#define ROMEN_WRITE_MASK 0x00
#define ROMEN_READ_MASK  ROMEN_WRITE_MASK
#define ROMEN_DAT        __ARR(ROMEN)

//TODO: these
// #define HDMA1_REG        0x51
// #define HDMA2_REG        0x52
// #define HDMA3_REG        0x53
// #define HDMA4_REG        0x54
// #define HDMA5_REG        0x55
// #define RP_REG           0x56
// #define BCPS_REG         0x68
// #define BCPD_REG         0x69
// #define OCPS_REG         0x6A
// #define OCPD_REG         0x6B

#define SVBK_REG        0x70
#define SVBK_WRITE_MASK 0x07
#define SVBK_READ_MASK  SVBK_WRITE_MASK
#define SVBK_DAT        __ARR(SVBK)

// Interrupt Enable
#define IE_REG           0xFF
#define IE_WRITE_MASK    0x1F
#define IE_READ_MASK     IE_WRITE_MASK
#define IE_DAT           _ARR(IE)