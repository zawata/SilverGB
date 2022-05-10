#include <cassert>
#include <functional>

#include <nowide/iostream.hpp>

#include "gba_core/cpu.hpp"

#include "gba_core/io.hpp"
#include "util/bit.hpp"
#include "util/file.hpp"
#include "util/flags.hpp"

u32 registers[16];
u32 cpsr_reg;

#define REG(X) (registers[(X)])

#define LR 14
#define PC 15

#define REG_LR (REG(LR))
#define REG_PC (REG(PC))

#define BOOL(x) (!(!(x)))
#define BitTest(arg,posn)       BOOL((arg) & (1L << (posn)))
#define BitSet(arg,posn)        ((arg) | (1L << (posn)))
#define BitChange(arg,posn,stt) (((arg) & ~(1UL << (posn))) | ((stt) << (posn)))
#define BitReset(arg,posn)      ((arg) & ~(1L << (posn)))
#define BitFlip(arg,posn)       ((arg) ^ (1L << (posn)))

#define make_reg_funcs(name, bit) \
 __force_inline bool get_##name() { return BitTest(cpsr_reg, bit); } \
 __force_inline void set_##name() { cpsr_reg = BitSet(cpsr_reg, bit); } \
 __force_inline void change_##name(bool state) { cpsr_reg = BitChange(cpsr_reg, bit, (u8)state);} \
 __force_inline void reset_##name() { cpsr_reg = BitReset(cpsr_reg, bit); } \
 __force_inline void flip_##name() { cpsr_reg = BitFlip(cpsr_reg, bit); };

make_reg_funcs(CPSR_N, 31);
make_reg_funcs(CPSR_Z, 30);
make_reg_funcs(CPSR_C, 29);
make_reg_funcs(CPSR_V, 28);

make_reg_funcs(CPSR_I, 7);
make_reg_funcs(CPSR_F, 6);
make_reg_funcs(CPSR_T, 5);

__force_inline bool    check_carry_16(u16 x, u16 y,        u32 r) { return (x^y^r)   & 0x10000; }
__force_inline bool    check_carry_16(u16 x, u16 y, u16 z, u32 r) { return (x^y^z^r) & 0x10000; }
__force_inline bool check_overflow_16(s16 x, s16 y,        s32 r) { return (x^y^r)   & 0x8000; }
__force_inline bool check_overflow_16(s16 x, s16 y, s16 z, s32 r) { return (x^y^z^r) & 0x8000; }
__force_inline bool    check_carry_32(u32 x, u32 y,        u64 r) { return (x^y^r)   & 0x100000000; }
__force_inline bool    check_carry_32(u32 x, u32 y, u32 z, u64 r) { return (x^y^z^r) & 0x100000000; }
__force_inline bool check_overflow_32(s32 x, s32 y,        s64 r) { return (x^y^r)   & 0x80000000; }
__force_inline bool check_overflow_32(s32 x, s32 y, s32 z, s64 r) { return (x^y^z^r) & 0x80000000; }

auto arm_op_fields = Bit::BitFieldAccessor<u32>();

CPU::CPU(IO_Bus *io, bool bootrom_enabled) :
io(io) {
    REG_PC = 0;

    arm_op_fields
        // all data types
        .add_field("condition",      0xF0000000)
        .add_field("op_code",        0x0C000000)
        .add_field("is_imm",         1, 25)
        //data processing
        .add_field("dp_command",     0x03F00000)
        .add_field("dp_reg_N",       0x000F0000)
        .add_field("dp_reg_D",       0x0000F000)
        //data processing: reg
        .add_field("dpr_shift_type", 0x00000060)
        .add_field("dpr_reg_M",      0x0000000F)
        .add_field("dpr_shift_imm",  0x00000F80)
        .add_field("dpr_reg_S",      0x00000F00)
        //data processing: immediate
        .add_field("dpi_rotate",     0x00000F00)
        .add_field("dpi_immediate",  0x000000FF)
        //branch and exchange
        .add_field("bex_const",      0x0FFFFFF0)
        //PSR transfer
        .add_field("psr_const",      0x01FF0000)
        //multiply
        .add_field("mul_const",      0x000000F0)
        //Load/Store
        .add_field("ls_is_pre_idx",  1, 24)
        .add_field("ls_is_up",       1, 23)
        .add_field("ls_is_word",     1, 22)
        .add_field("ls_write_back",  1, 21)
        .add_field("ls_is_load",     1, 20)
        .add_field("ls_reg_N",       0x000F0000)
        .add_field("ls_reg_D",       0x0000F000);
}

CPU::~CPU() {}

void CPU::tick() {
    execute();
}

u8 CPU::decode_thumb(u16 op) {
    //TODO
    return 0;
}


const char * cond_str[] = {
    "EQ",
    "NE",
    "CS",
    "CC",
    "MI",
    "PL",
    "VS",
    "VC",
    "HI",
    "LS",
    "GE",
    "LT",
    "GT",
    "LE",
    "",
    "ERROR"
};

bool cond_equal()                        { return get_CPSR_Z(); }
bool cond_not_equal()                    { return !cond_equal(); }
bool cond_carry_set()                    { return get_CPSR_C(); }
bool cond_carry_clear()                  { return !cond_carry_set(); }
bool cond_minus()                        { return get_CPSR_N(); }
bool cond_plus()                         { return !cond_minus(); }
bool cond_overflow()                     { return get_CPSR_V(); }
bool cond_no_overflow()                  { return !cond_overflow(); }
bool cond_unsigned_higher()              { return cond_carry_set() && cond_not_equal(); }
bool cond_unsigned_lower_or_same()       { return !cond_unsigned_higher(); }
bool cond_signed_greater_than_or_equal() { return get_CPSR_N() == get_CPSR_V(); }
bool cond_signed_less_than()             { return !cond_signed_greater_than_or_equal(); }
bool cond_signed_greater_than()          { return cond_not_equal() && cond_signed_greater_than_or_equal(); }
bool cond_signed_less_than_or_equal()    { return !cond_signed_greater_than(); }
bool cond_always()                       { return true; }
bool cond_never()                        { return false; }

bool (*conditions[])(void) = {
    cond_equal,
    cond_not_equal,
    cond_carry_set,
    cond_carry_clear,
    cond_minus,
    cond_plus,
    cond_overflow,
    cond_no_overflow,
    cond_unsigned_higher,
    cond_unsigned_lower_or_same,
    cond_signed_greater_than_or_equal,
    cond_signed_less_than,
    cond_signed_greater_than,
    cond_signed_less_than_or_equal,
    cond_always,
    cond_never // unpredictable in ARMv4, but whatever
};

void CPU::prefetch() {
    nowide::cout << "prefetch() " << as_hex(REG_PC) << std::endl;
    op2 = op1;
    op1 = io->read(REG_PC);
}

void CPU::prefetch(u32 dest) {
    nowide::cout << "prefetch(u32) " << getchar() << std::endl;
    op2 = op1;
    op1 = io->read(dest);
}

void CPU::execute() {
    #define MUL_CONSTANT 0b1001
    #define BEX_CONSTANT 0b000100101111111111110001

    #define MRS_C_CONSTANT  0b100001111
    #define MRS_P_CONSTANT  0b101001111
    #define MSR_C_CONSTANT  0b100101001
    #define MSR_P_CONSTANT  0b101101001
    #define MSRF_C_CONSTANT 0b100101000
    #define MSRF_P_CONSTANT 0b101101000

    u32 word = op2;

    bool is_mul = arm_op_fields.get(word, "mul_const") == MUL_CONSTANT;
    bool is_swp = is_mul && arm_op_fields.get(word, "dpr_reg_S") == 0x0000;
    bool is_bex = arm_op_fields.get(word, "bex_const") == BEX_CONSTANT;

    const auto IMM_MODE = 0x20;
    #define cmd_op(op_name) (op_data_proc_cmd_##op_name)

    switch(arm_op_fields.get(word, "dp_command")) {
    // Data PRocessing Ops
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  0  [OpCode   ] S  [Rn       ] [Rd       ] [Shift                ] [Rm       ] reg/shift

    // 00
    case cmd_op(AND): //fallthrough
        //MUL
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // short        [Cond     ] 0  0  0  0  0  0  0  0  [Rd       ] [Rn       ] [Rs       ] 1  0  0  1  [Rm       ]
    // 01
    case cmd_op(ANDS):
        //MULS
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // short        [Cond     ] 0  0  0  0  0  0  0  1  [Rd       ] [Rn       ] [Rs       ] 1  0  0  1  [Rm       ]
        if(is_mul) {
            decode_mul_op(word);
            break;
        }

        decode_dp_op(word);
        break;
    // 02
    case cmd_op(EOR): //fallthrough
        //MLA
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // short        [Cond     ] 0  0  0  0  0  0  1  0  [Rd       ] [Rn       ] [Rs       ] 1  0  0  1  [Rm       ]
    // 03
    case cmd_op(EORS):
        //MLAS
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // short        [Cond     ] 0  0  0  0  0  0  1  1  [Rd       ] [Rn       ] [Rs       ] 1  0  0  1  [Rm       ]
        if(is_mul) {
            decode_mul_op(word);
            break;
        }

        decode_dp_op(word);
        break;
    // 04
    case cmd_op(SUB): //fallthrough
    // 05
    case cmd_op(SUBS):
        decode_dp_op(word);
        break;
    // 06
    case cmd_op(RSB): //fallthrough
    // 07
    case cmd_op(RSBS):
        decode_dp_op(word);
        break;
    // 08
    case cmd_op(ADD): //fallthrough
        //UMULL
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  0  0  0  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
    // 09
    case cmd_op(ADDS):
        //UMULLS
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  0  0  1  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
        if(is_mul) {
            decode_mul_op(word);
            break;
        }

        decode_dp_op(word);
        break;
    // 0A
    case cmd_op(ADC): //fallthrough
        //UMLAL
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  0  1  0  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
    // 0B
    case cmd_op(ADCS):
        //UMLALS
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  0  1  1  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
        if(is_mul) {
            decode_mul_op(word);
            break;
        }

        decode_dp_op(word);
        break;
    // 0C
    case cmd_op(SBC): //fallthrough
        //MULL - signed
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  1  0  0  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
    // 0D
    case cmd_op(SBCS):
        //MULLS - signed
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  1  0  1  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
        if(is_mul) {
            decode_mul_op(word);
            break;
        }

        decode_dp_op(word);
        break;
    // 0E
    case cmd_op(RSC): //fallthrough
        //MLAL - signed
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  1  1  0  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
    // 0F
    case cmd_op(RSCS):
        //MLALS - signed
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // long         [Cond     ] 0  0  0  0  1  1  1  1  [H-Rd     ] [L-Rd     ] [Rs       ] 1  0  0  1  [Rm       ]
        if(is_mul) {
            decode_mul_op(word);
            break;
        }

        decode_dp_op(word);
        break;
    // 10
    case cmd_op(noset1): //fallthrough
        // SWP - word
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // swap         [Cond     ] 0  0  0  1  0  0  0  0  [Rn       ] [Rd       ] 0  0  0  0  1  0  0  1  [Rm       ]
        if(is_swp) {
            decode_swap_op(word);
            break;
        }

        // MRS CPSR
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  0  1  0  0  0  0  1  1  1  1  [Rd       ] 0  0  0  0  0  0  0  0  0  0  0  0
    // 11
    case cmd_op(TST):
        decode_dp_op(word);
        break;
    // 12
    case cmd_op(noset2): //fallthrough
        // Branch and exchange
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  0  1  0  0  1  0  1  1  1  1  1  1  1  1  1  1  1  1  0  0  0  1  [Rn       ]
        if(is_bex) {
            decode_bex_op(word);
            break;
        }

        // MSR CPSR
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  0  1  0  0  1  0  1  0  0  1  1  1  1  1  0  0  0  0  0  0  0  0  [Rm       ]
        // flags only   [Cond     ] 0  0  0  1  0  0  1  0  1  0  0  0  1  1  1  1  0  0  0  0  0  0  0  0  [Rm       ]
    // 13
    case cmd_op(TEQ):
        decode_dp_op(word);
        break;
    // 14
    case cmd_op(noset3): //fallthrough
        // SWP - byte
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        // swap         [Cond     ] 0  0  0  1  0  1  0  0  [Rn       ] [Rd       ] 0  0  0  0  1  0  0  1  [Rm       ]
        if(is_swp) {
            decode_swap_op(word);
            break;
        }

        // MRS SPSR
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  0  1  0  1  0  0  1  1  1  1  [Rd       ] 0  0  0  0  0  0  0  0  0  0  0  0
    // 15
    case cmd_op(CMP):
        decode_dp_op(word);
        break;
    // 16
    case cmd_op(noset4): //fallthrough
        // MSR SPSR
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  0  1  0  1  1  0  1  0  0  1  1  1  1  1  0  0  0  0  0  0  0  0  [Rm       ]
        // flags only   [Cond     ] 0  0  0  1  0  1  1  0  1  0  0  0  1  1  1  1  0  0  0  0  0  0  0  0  [Rm       ]
    // 17
    case cmd_op(CMN):
        decode_dp_op(word);
        break;
    // 18
    case cmd_op(ORR): //fallthrough
    // 19
    case cmd_op(ORRS):
        decode_dp_op(word);
        break;
    // 1A
    case cmd_op(MOV): //fallthrough
    // 1B
    case cmd_op(MOVS):
        decode_dp_op(word);
        break;
    // 1C
    case cmd_op(BIC): //fallthrough
    // 1D
    case cmd_op(BICS):
        decode_dp_op(word);
        break;
    // 1E
    case cmd_op(MVN): //fallthrough
    // 1F
    case cmd_op(MVNS):
        decode_dp_op(word);
        break;

    // Data Processing Ops
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  1  [OpCode   ] S  [Rn       ] [Rd       ] [Immediate                        ] immediate
    // 20
    case IMM_MODE | cmd_op(AND): //fallthrough
    // 21
    case IMM_MODE | cmd_op(ANDS):
        decode_dp_op(word);
        break;
    // 22
    case IMM_MODE | cmd_op(EOR): //fallthrough
    // 23
    case IMM_MODE | cmd_op(EORS):
        decode_dp_op(word);
        break;
    // 24
    case IMM_MODE | cmd_op(SUB): //fallthrough
    // 25
    case IMM_MODE | cmd_op(SUBS):
        decode_dp_op(word);
        break;
    // 26
    case IMM_MODE | cmd_op(RSB): //fallthrough
    // 27
    case IMM_MODE | cmd_op(RSBS):
        decode_dp_op(word);
        break;
    // 28
    case IMM_MODE | cmd_op(ADD): //fallthrough
    // 29
    case IMM_MODE | cmd_op(ADDS):
        decode_dp_op(word);
        break;
    // 2A
    case IMM_MODE | cmd_op(ADC): //fallthrough
    // 2B
    case IMM_MODE | cmd_op(ADCS):
        decode_dp_op(word);
        break;
    // 2C
    case IMM_MODE | cmd_op(SBC): //fallthrough
    // 2D
    case IMM_MODE | cmd_op(SBCS):
        decode_dp_op(word);
        break;
    // 2E
    case IMM_MODE | cmd_op(RSC): //fallthrough
    // 2F
    case IMM_MODE | cmd_op(RSCS):
        decode_dp_op(word);
        break;
    // 30
    case IMM_MODE | cmd_op(noset1): //fallthrough
        decode_swi(word);
    // 31
    case IMM_MODE | cmd_op(TST):
        decode_dp_op(word);
        break;
    // 32
    case IMM_MODE | cmd_op(noset2): //fallthrough
        // MSR CPSR - flags only
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  1  1  0  0  1  0  1  0  0  0  1  1  1  1  [Rot      ] [immediate            ]
    // 33
    case IMM_MODE | cmd_op(TEQ):
        decode_dp_op(word);
        break;
    // 34
    case IMM_MODE | cmd_op(noset3): //fallthrough
        decode_swi(word);
    // 35
    case IMM_MODE | cmd_op(CMP):
        decode_dp_op(word);
        break;
    // 36
    case IMM_MODE | cmd_op(noset4): //fallthrough
        // MSR CPSR - flags only
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  0  1  1  0  0  1  0  1  0  0  0  1  1  1  1  [Rot      ] [immediate            ]
    // 37
    case IMM_MODE | cmd_op(CMN):
        decode_dp_op(word);
        break;
    // 38
    case IMM_MODE | cmd_op(ORR): //fallthrough
    // 39
    case IMM_MODE | cmd_op(ORRS):
        decode_dp_op(word);
        break;
    // 3A
    case IMM_MODE | cmd_op(MOV): //fallthrough
    // 3B
    case IMM_MODE | cmd_op(MOVS):
        decode_dp_op(word);
        break;
    // 3C
    case IMM_MODE | cmd_op(BIC): //fallthrough
    // 3D
    case IMM_MODE | cmd_op(BICS):
        decode_dp_op(word);
        break;
    // 3E
    case IMM_MODE | cmd_op(MVN): //fallthrough
    // 3F
    case IMM_MODE | cmd_op(MVNS):
        decode_dp_op(word);
        break;

    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
    case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
    case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        // TODO: what is this for?

    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
    case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
    case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
        // Single Data Transfer
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  1  1  P  U  S  W  L  [Rn       ] [Rd       ] [Offset                           ]

        // Undefined
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 0  1  1  [                                                         ] 1  [         ]

        if(word & 0x10) {
            decode_swi(word);
        } else {
            decode_load_store_op(word);
        }
        break;

    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
    case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
    case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
    case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
        // Block Data Transfer
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 1  0  0  P  U  S  W  L  [Rn       ] [Register List                                ]
        decode_load_store_op(word);
        break;

    case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
    case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
    case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
    case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        // Branch
        //              31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
        //              [Cond     ] 1  0  1  L  [Offset                                                               ]
        decode_branch_op(word);
        break;
    case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7:
    case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
    case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xDC: case 0xDD: case 0xDE: case 0xDF:
    case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
    case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
        decode_coproc_op(word);
        break;

    case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
    case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
        decode_swi(word);
        break;
    }
}

inline void CPU::decode_dp_op(u32 word) {
    bool (*condition)(void) = conditions[arm_op_fields.get(word, "condition")];
    bool immediate =  arm_op_fields.get<bool>(word, "is_imm");
    u8 command =      arm_op_fields.get(word, "dp_command");
    u8 reg_N =        arm_op_fields.get(word, "dp_reg_N");
    u8 reg_D =        arm_op_fields.get(word, "dp_reg_D");

    if(immediate) {
        nowide::cout << "immediate" << std::endl;
        data_proc_immediate(
            condition,
            (op_data_proc_cmd_t)command,
            reg_N,
            reg_D,
            arm_op_fields.get(word, "dpi_rotate"),
            arm_op_fields.get(word, "dpi_immediate"));
    } else {
        u8 shift_type = arm_op_fields.get(word, "dpr_shift_type");
        u8 reg_M      = arm_op_fields.get(word, "dpr_reg_M");
        if(Bit::test(word, 4)) {
            data_proc_shifted_reg(
                    condition,
                    (op_data_proc_cmd_t)command,
                    reg_N,
                    reg_D,
                    arm_op_fields.get(word, "dpr_reg_S"),
                    (op_data_proc_shift_t)shift_type,
                    reg_M);
        } else {
            u8 shift_amount = arm_op_fields.get(word, "dpr_shift_imm");
            data_proc_reg(
                    condition,
                    (op_data_proc_cmd_t)command,
                    reg_N,
                    reg_D,
                    shift_amount,
                    (op_data_proc_shift_t)shift_type,
                    reg_M);
        }
    }
}

inline void CPU::decode_mul_op(u32 word) {
    //TODO
}

inline void CPU::decode_load_store_op(u32 word) {
    nowide::cout << "ls" << std::endl;
    bool (*condition)(void) = conditions[arm_op_fields.get(word, "condition")];
    bool is_imm =     arm_op_fields.get<bool>(word, "is_imm");
    bool is_pre_idx = arm_op_fields.get<bool>(word, "ls_is_pre_idx");
    bool is_up =      arm_op_fields.get<bool>(word, "ls_is_up");
    bool is_word =    arm_op_fields.get<bool>(word, "ls_is_word");
    bool write_back = arm_op_fields.get<bool>(word, "ls_write_back");
    bool is_load =    arm_op_fields.get<bool>(word, "is_load");
    u8   reg_N =      arm_op_fields.get<u8>(word, "reg_N");
    u8   reg_D =      arm_op_fields.get<u8>(word, "reg_D");

    if(is_imm) {
        std::invoke((is_load ? &CPU::load_immediate : &CPU::store_immediate), this,
                condition,
                is_pre_idx,
                is_up,
                is_word,
                write_back,
                reg_N,
                reg_D,
                arm_op_fields.get(word, "ls_immediate"));
    } else {
        std::invoke((is_load ? &CPU::load_shifted_reg : &CPU::store_shifted_reg), this,
                condition,
                is_pre_idx,
                is_up,
                is_word,
                write_back,
                reg_N,
                reg_D,
                arm_op_fields.get(word, "ls_reg_S"),
                arm_op_fields.get<op_data_proc_shift_t>(word, "dpr_shift_type"),
                arm_op_fields.get<u8>(word, "dpr_reg_M"));
    }
}

inline void CPU::decode_branch_op(u32 word) {
    bool (*condition)(void) = conditions[arm_op_fields.get(word, "condition")];
    bool link = arm_op_fields.get<bool>(word, "is_link");
    s32 immediate = Bit::sign_extend(arm_op_fields.get(word, "imm"), 23);

    branch(condition, link, immediate);
}

inline void CPU::decode_bex_op(u32 word) {
    bool (*condition)(void) = conditions[arm_op_fields.get(word, "condition")];
    u8 reg_N = arm_op_fields.get(word, "reg_N");

    branch_and_exchange(
            condition,
            reg_N);
}

inline void throw_data_abort() { /* TODO */ }

inline u32 CPU::calc_shift_operand(op_data_proc_shift_t shift_type, u8 amount, u8 reg_M, bool *carry_out) {
    switch(shift_type) {
    case op_data_proc_shift_LSL:
        return Bit::shift_with_carry<u32>::shift_left(REG(reg_M), amount, get_CPSR_C(), carry_out);
    case op_data_proc_shift_LSR:
        return Bit::shift_with_carry<u32>::shift_right(REG(reg_M), amount, carry_out);
    case op_data_proc_shift_ASR:
        return Bit::shift_with_carry<u32>::arithmetic_shift_right(REG(reg_M), amount, carry_out);
    case op_data_proc_shift_ROR:
        return Bit::shift_with_carry<u32>::rotate_right(REG(reg_M), amount, carry_out);
    }
}

inline void CPU::execute_data_processing_command(
    CPU::op_data_proc_cmd_t command,
    u8 reg_N,
    u8 reg_D,
    u32 shift_operand,
    bool shift_carry_out
) {
    nowide::cout << "dp: " << (int)command << std::endl;
    bool set_flags = Bit::test(command, 0);
    switch(command >> 1) {
    case op_data_proc_cmd_AND: {
        nowide::cout << "AND" << std::endl;
        REG(reg_D) = REG(reg_N) & shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case op_data_proc_cmd_EOR: {
        nowide::cout << "EOR" << std::endl;
        REG(reg_D) = REG(reg_N) ^ shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case op_data_proc_cmd_SUB: {
        nowide::cout << "SUB" << std::endl;
        REG(reg_D) = REG(reg_N) - shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand));
        }
        break;
    }
    case op_data_proc_cmd_RSB: {
        nowide::cout << "RSB" << std::endl;
        REG(reg_D) = shift_operand - REG(reg_N);

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, shift_operand - REG(reg_N)));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, shift_operand - REG(reg_N)));
        }
        break;
    }
    case op_data_proc_cmd_ADD: {
        nowide::cout << "ADD" << std::endl;
        REG(reg_D) = REG(reg_N) + shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(check_carry_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
        }
        break;
    }
    case op_data_proc_cmd_ADC: {
        nowide::cout << "ADC" << std::endl;
        REG(reg_D) = REG(reg_N) + shift_operand + (get_CPSR_C() ? 1 : 0);

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(check_carry_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
        }
        break;
    }
    case op_data_proc_cmd_SBC: {
        nowide::cout << "SBC" << std::endl;
        REG(reg_D) = REG(reg_N) - shift_operand - (!get_CPSR_C() ? 1 : 0);

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand - (!get_CPSR_C() ? 1 : 0)));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand - (!get_CPSR_C() ? 1 : 0)));
        }
        break;
    }
    case op_data_proc_cmd_RSC: {
        nowide::cout << "RSC" << std::endl;
        REG(reg_D) = shift_operand - REG(reg_N) - (!get_CPSR_C() ? 1 : 0);

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, shift_operand - REG(reg_N) - (!get_CPSR_C() ? 1 : 0)));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, shift_operand - REG(reg_N) - (!get_CPSR_C() ? 1 : 0)));
        }
        break;
    }
    case op_data_proc_cmd_TST: {
        nowide::cout << "TST" << std::endl;
        assert(set_flags); //TODO: remove this, just curious :P

        u32 alu_out = REG(reg_N) - shift_operand;

        change_CPSR_N(Bit::test(alu_out, 31));
        change_CPSR_Z(alu_out == 0);
        change_CPSR_C(shift_carry_out);
        // V unaffected
        break;
    }
    case op_data_proc_cmd_TEQ: {
        nowide::cout << "TEQ" << std::endl;
        assert(set_flags); //TODO: remove this, just curious :P

        u32 alu_out = REG(reg_N) - shift_operand;

        change_CPSR_N(Bit::test(alu_out, 31));
        change_CPSR_Z(alu_out == 0);
        change_CPSR_C(shift_carry_out);
        // V unaffected
        break;
    }
    case op_data_proc_cmd_CMP: {
        nowide::cout << "CMP" << std::endl;
        assert(set_flags); //TODO: remove this, just curious :P

        u32 alu_out = REG(reg_N) - shift_operand;

        change_CPSR_N(Bit::test(alu_out, 31));
        change_CPSR_Z(alu_out == 0);
        change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand));
        change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand));
        break;
    }
    case op_data_proc_cmd_CMN: {
        nowide::cout << "CMN" << std::endl;
        assert(set_flags); //TODO: remove this, just curious :P

        u32 alu_out = REG(reg_N) + shift_operand;

        change_CPSR_N(Bit::test(alu_out, 31));
        change_CPSR_Z(alu_out == 0);
        change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
        change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
        break;
    }
    case op_data_proc_cmd_ORR: {
        nowide::cout << "ORR" << std::endl;
        REG(reg_D) = REG(reg_N) | shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case op_data_proc_cmd_MOV: {
        nowide::cout << "MOV" << std::endl;
        REG(reg_N) = shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_N), 31));
            change_CPSR_Z(REG(reg_N) == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case op_data_proc_cmd_BIC: {
        nowide::cout << "BIC" << std::endl;
        REG(reg_D) = REG(reg_N) & ~shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_D), 31));
            change_CPSR_Z(REG(reg_D) == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case op_data_proc_cmd_MVN: {
        nowide::cout << "MVN" << std::endl;
        REG(reg_N) = ~shift_operand;

        if(set_flags) {
            change_CPSR_N(Bit::test(REG(reg_N), 31));
            change_CPSR_Z(REG(reg_N) == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    }
}

inline void CPU::execute_load_store_command(
    bool load,
    bool pre_index,
    bool add,
    bool word,
    bool write,
    u8 reg_N,
    u8 reg_D,
    u32 offset
) {
    #define _calc_sub_idx(reg, offset) (REG(reg_N) - offset)
    #define _calc_add_idx(reg, offset) (REG(reg_N) + offset)
    #define calc_auto_idx(reg, offset, is_add) (is_add ? _calc_add_idx(reg, offset) : _calc_sub_idx(reg, offset))

    u32 index;
    if(pre_index) {
        index = calc_auto_idx(reg_N, offset, add);
        if(word && (index % 4) != 0) {
            throw_data_abort();
        }
        if(write) {
            REG(reg_N) = index;
        }
    } else {
        index = REG(reg_N);
    }

    //cycle 3
    io->tick();
    prefetch();
    REG(reg_D) = io->read(index) & (word ? 0xFFFFFFFF : 0xFF);

    if(!pre_index) {
        index = calc_auto_idx(reg_N, offset, add);

        // base reg is always written on a post-idx?
        REG(reg_N) = index;
    }

}

// Instruction Timings:
//
// Cycle types:
// nMREQ  SEQ  Cycle Types:
// 0     0     Non-sequential (N-cycle)
//               - 
// 0     1     Sequential (S-cycle)
//               - 
// 1     0     Internal (I-cycle)
//               - no prefetch
// 1     1     Coprocessor register transfer (C-cycle)
//               - not used

void CPU::data_proc_reg(
    bool (*condition)(void),
    op_data_proc_cmd_t command,
    u8 reg_N,
    u8 reg_D,
    u8 shift_amount,
    op_data_proc_shift_t shift_type,
    u8 reg_M
) {
    // normal    1  pc+2L  i  0  (pc+2L)  S 0
    //              pc+3L
    // dest=pc   1  pc+2L  i  0  (pc+2L)  N 0
    //           2  alu    i  0  (alu)    S 0
    //           3  alu+L  i  0  (alu+L)  S 0
    //              alu+2L

    //cycle 1
    io->tick();
    prefetch();
    if(condition()) {
        bool shift_carry_out;
        u32 shift_operand = calc_shift_operand(shift_type, shift_amount, reg_M, &shift_carry_out);
        execute_data_processing_command(
            command,
            reg_N,
            reg_D,
            shift_operand,
            shift_carry_out);

        if(reg_D == PC) {
            //cycle 2
            io->tick();
            prefetch();

            //cycle 3
            io->tick();
            prefetch();
        }
    }
}

void CPU::data_proc_shifted_reg(
    bool (*condition)(void),
    op_data_proc_cmd_t command,
    u8 reg_N,
    u8 reg_D,
    u8 reg_S,
    op_data_proc_shift_t shift_type,
    u8 reg_M
) {
    // shift(Rs) 1  pc+2L  i  0 (pc+2L)   I 0
    //           2  pc+3L  i  0 -         S 1
    //              pc+3L
    // shift(Rs) 1  pc+8   2  0 (pc+8)    I 0
    // dest=pc   2  pc+12  2  0 -         N 1
    //           3  alu    2  0 (alu)     S 0
    //           4  alu+4  2  0 (alu+4)   S 0
    //              alu+8

    //cycle 1
    io->tick();
    prefetch();
    if(condition()) {
        u32 shift_amount = REG(reg_S);

        //cycle 2
        io->tick();
        // internal cycle, no prefetch

        bool shift_carry_out;
        u32 shift_operand = calc_shift_operand(shift_type, shift_amount, reg_M, &shift_carry_out);
        execute_data_processing_command(
            command,
            reg_N,
            reg_D,
            shift_operand,
            shift_carry_out);

        if(reg_D == PC) {
            //cycle 2
            io->tick();
            prefetch();

            //cycle 3
            io->tick();
            prefetch();
        }
    }
}

void CPU::data_proc_immediate(
    bool (*condition)(void),
    op_data_proc_cmd_t command,
    u8 reg_N,
    u8 reg_D,
    u8 rotate,
    u8 immediate
) {
    // normal    1  pc+2L  i  0  (pc+2L)  S 0
    //              pc+3L
    // dest=pc   1  pc+2L  i  0  (pc+2L)  N 0
    //           2  alu    i  0  (alu)    S 0
    //           3  alu+L  i  0  (alu+L)  S 0
    //              alu+2L

    //cycle 1
    io->tick();
    prefetch();
    if(condition()) {
        bool shift_carry_out;
        u32 shift_operand = Bit::shift_with_carry<u32>::rotate_right(immediate, 1 << rotate, &shift_carry_out);
        execute_data_processing_command(
            command,
            reg_N,
            reg_D,
            shift_operand,
            shift_carry_out);

        if(reg_D == PC) {
            //cycle 2
            io->tick();
            prefetch();

            //cycle 3
            io->tick();
            prefetch();
        }
    }
}

void CPU::load_shifted_reg(
    bool (*condition)(void),
    bool pre_index,
    bool add,
    bool word,
    bool write,
    u8 reg_N,
    u8 reg_D,
    u8 shift_amount,
    op_data_proc_shift_t shift_type,
    u8 reg_M
) {
    // normal    1  pc+2L  i  0  (pc+2L) N 0 c
    //           2  alu    s  0  (alu)   I 1 d
    //           3  pc+3L  i  0  -       S 1 c
    //              pc+3L
    // dest=pc   1  pc+8   2  0  (pc+8)  N 0 c
    //           2  alu    0     pc’     I 1 d
    //           3  pc+12  2  0  -       N 1 c
    //           4  pc’    2  0  (pc’)   S 0 c
    //           5  pc’+4  2  0  (pc’+4) S 0 c
    //              pc’+8

    //cycle 1
    io->tick();
    prefetch();
    if(condition()) {

        //cycle 2
        io->tick();
        // internal cycle, no prefetch

        //this will perform cycle 3
        execute_load_store_command(
            true,
            pre_index,
            add,
            word,
            write,
            reg_N,
            reg_D,
            calc_shift_operand(shift_type, shift_amount, reg_M, nullptr));

        if(reg_D == PC) {
            //cycle 4
            io->tick();
            prefetch();

            //cycle 5
            io->tick();
            prefetch();
        }
    }
}


void CPU::store_shifted_reg(
    bool (*condition)(void),
    bool pre_index,
    bool add,
    bool word,
    bool write,
    u8 reg_N,
    u8 reg_D,
    u8 shift_amount,
    op_data_proc_shift_t shift_type,
    u8 reg_M
) {
    // normal    1  pc+2L  i  0  (pc+2L) N 0 c
    //           2  alu    s  1  Rd      N 1 d
    //              pc+3L

    //cycle 1
    io->tick();
    prefetch();
    if(condition()) {
        u32 shift_operand = calc_shift_operand(shift_type, shift_amount, reg_M, nullptr);

        if(reg_D == PC) {
            //cycle 2
            io->tick();
            prefetch();

            //cycle 3
            io->tick();
            prefetch();
        }
    }
}

void CPU::load_store_immediate(
    bool (*condition)(void),
    bool pre_index,
    bool add,
    bool word,
    bool write,
    bool is_load,
    u8 reg_N,
    u8 reg_D,
    u16 immediate
) {
    // normal    1  pc+2L  i  0  (pc+2L) N 0 c
    //           2  alu    s  0  (alu)   I 1 d
    //           3  pc+3L  i  0  -       S 1 c
    //              pc+3L
    // dest=pc   1  pc+8   2  0  (pc+8)  N 0 c
    //           2  alu    0     pc’     I 1 d
    //           3  pc+12  2  0  -       N 1 c
    //           4  pc’    2  0  (pc’)   S 0 c
    //           5  pc’+4  2  0  (pc’+4) S 0 c
    //              pc’+8

    //cycle 1
    io->tick();
    prefetch();
    if(condition()) {

        //cycle 2
        io->tick();
        // internal cycle, no prefetch

        //this will perform cycle 3
        execute_load_store_command(
            true,
            pre_index,
            add,
            word,
            write,
            reg_N,
            reg_D,
            immediate);

        if(reg_D == PC) {
            //cycle 4
            io->tick();
            prefetch();

            //cycle 5
            io->tick();
            prefetch();
        }
    }
}

void CPU::branch(
    bool (*condition)(void),
    bool link,
    s32 immediate
) {
    // 2S + 1N
    // 1  pc+2L   i  0  (pc+2L)  N 0
    // 2  alu     i  0  (alu)    S 0
    // 3  alu+L   i  0  (alu+L)  S 0
    //    alu+2L

    //cycle 1
    io->tick();
    prefetch();
    if(condition) {
        //TODO: not exactly sure where PC gets set. its either:
        // - in cycle 1 so the LR register is updated with a temp former PC value in cycle 2
        // - in cycle 2 so the second 2 prefetches are performed from PC
        // - in cycle 3 so the prefetches are computed from the alu result
        // the last one seems cleanest imo

        u32 dest = REG_PC + immediate;

        //cycle 2
        io->tick();
        prefetch(dest);
        if(link) {
            REG_LR = REG_PC;
        }

        //cycle 3
        io->tick();
        prefetch(dest + 4);
        if(link) {
            REG_LR -= 4;
        }
        REG_PC = dest << 2;
    }
}

//TODO: this function
void CPU::branch_and_exchange(
    bool (*condition)(void),
    s32 immediate
) {
    // 1  pc+2L   i  0  (pc+2L)  N 0
    // 2  alu     i  0  (alu)    S 0
    // 3  alu+L   i  0  (alu+L)  S 0
    //    alu+2L

    //cycle 1
    io->tick();
    prefetch();
    if(condition) {
        //TODO: not exactly sure where PC gets set. its either:
        // - in cycle 1 so the LR register is updated with a temp former PC value in cycle 2
        // - in cycle 2 so the second 2 prefetches are performed from PC
        // - in cycle 3 so the prefetches are computed from the alu result
        // the last one seems cleanest imo

        u32 dest = REG_PC + immediate;

        //cycle 2
        io->tick();
        prefetch(dest);
        if(link) {
            REG_LR = REG_PC;
        }

        //cycle 3
        io->tick();
        prefetch(dest + 4);
        if(link) {
            REG_LR -= 4;
        }
        REG_PC = dest << 2;
    }
}