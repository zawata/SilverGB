#include <cassert>

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


CPU::CPU(IO_Bus *io, bool bootrom_enabled) :
io(io) {
    REG_PC = 0;
}

CPU::~CPU() {}

void CPU::tick() {
    execute();
}

u8 CPU::decode_thumb(u16 op) {
    //TODO
    return 0;
}


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
    std::cout << "prefetch " << as_hex(REG_PC) << std::endl;
    op2 = op1;
    op1 = io->read(REG_PC);
}

void CPU::prefetch(u32 dest) {
    op2 = op1;
    op1 = io->read(dest);
}

void CPU::execute() {
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

    u32 word = op2;

    auto op_fields = Bit::BitFieldAccessor<u32>()
        // all data types
        .add_field("condition",      0xF0000000)
        .add_field("op_code",        0x0C000000)
        .add_field("is_imm",         1, 25)
        //data processing
        .add_field("dp_set_flags",   1, 20)
        .add_field("dp_command",     0x01E00000)
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
        //multiply
        .add_field("mul_const",      0x000000F0)
        //Load/Store
        .add_field("ls_index",       1, 24)
        .add_field("ls_is_pre_idx",  1, 23)
        .add_field("ls_is_up",       1, 22)
        .add_field("ls_is_word",     1, 21)
        .add_field("ls_is_write",    1, 20)
        .add_field("ls_is_load",     1, 19)
        .add_field("ls_reg_N",       0x000F0000)
        .add_field("ls_reg_D",       0x0000F000);

    #define MUL_CONSTANT 0b1001

    std::cout << (std::string)op_fields << std::endl;


    bool (*condition)(void) = conditions[op_fields.get(word, "condition")];

    switch(op_fields.get(word, "op_code")) {
        case op_code_DATA_PROCESSING: {
            std::cout << "dp" << std::endl;
            bool immediate =  op_fields.get<bool>(word, "is_imm");
            bool set_flags =  op_fields.get<bool>(word, "dp_set_flags");
            u8 command =      op_fields.get(word, "dp_command");
            u8 reg_N =        op_fields.get(word, "dp_reg_N");
            u8 reg_D =        op_fields.get(word, "dp_reg_D");

            if(immediate) {
                std::cout << "immediate" << std::endl;
                data_proc_immediate(
                    condition(),
                    (op_data_proc_cmd_t)command,
                    set_flags,
                    reg_N,
                    reg_D,
                    op_fields.get(word, "dpi_rotate"),
                    op_fields.get(word, "dpi_immediate"));
            //special case: if the bits 4-7 are set to 0b1001, then it's not a dp instruction, it's a multiply
            } else if(op_fields.get(word, "mul_const") == MUL_CONSTANT) {
                std::cout << "MULTIPLY" << std::endl;
            } else {
                u8 shift_type = op_fields.get(word, "dpr_shift_type");
                u8 reg_M      = op_fields.get(word, "dpr_reg_M");
                if(Bit::test(word, 4)) {
                    data_proc_shifted_reg(
                            condition(),
                            (op_data_proc_cmd_t)command,
                            set_flags,
                            reg_N,
                            reg_D,
                            op_fields.get(word, "dpr_reg_S"),
                            (op_data_proc_shift_t)shift_type,
                            reg_M);
                } else {
                    u8 shift_amount = op_fields.get(word, "dpr_shift_imm");
                    data_proc_reg(
                            condition(),
                            (op_data_proc_cmd_t)command,
                            set_flags,
                            reg_N,
                            reg_D,
                            shift_amount,
                            (op_data_proc_shift_t)shift_type,
                            reg_M);
                }
            }

            break;
        }
        case op_code_LOAD_STORE: {
            std::cout << "ls" << std::endl;
            bool immediate =  op_fields.get<bool>(word, "is_imm");
            bool is_pre_idx = op_fields.get<bool>(word, "ls_is_pre_idx");
            bool is_up =      op_fields.get<bool>(word, "ls_is_up");
            bool is_word =    op_fields.get<bool>(word, "ls_is_word");
            bool is_write =   op_fields.get<bool>(word, "ls_is_write");
            bool is_load =    op_fields.get<bool>(word, "ls_is_load");
            u8   reg_N =      op_fields.get<u8>(word, "dp_reg_N");
            u8   reg_D =      op_fields.get<u8>(word, "dp_reg_D");

            if(immediate) {
                std::cout << "immediate" << std::endl;
                load_store_immediate(
                        condition(),
                        is_pre_idx,
                        is_up,
                        is_word,
                        is_write,
                        is_load,
                        reg_N,
                        reg_D,
                        op_fields.get(word, "ls_immediate"));
            } else {
                load_store_shifted_reg(
                        condition(),
                        is_pre_idx,
                        is_up,
                        is_word,
                        is_write,
                        is_load,
                        reg_N,
                        reg_D,
                        op_fields.get(word, "ls_reg_S"),
                        op_fields.get<op_data_proc_shift_t>(word, "dpr_shift_type"),
                        op_fields.get<u8>(word, "dpr_reg_M"));
            }

            break;
        }
        case op_code_BRANCH: {
            bool link = Bit::test(word, 24);
            s32 immediate = Bit::sign_extend(word & 0x00FFFFFF, 23);

            std::cout << "branch" << std::endl;

            branch(condition, Bit::test(word, 24), Bit::sign_extend(word & 0x00FFFFFF, 23));
        }
        case op_code_COPROCESSOR:
            std::cout << "coprocessor!" << std::endl;
        break;
    }
}

u32 CPU::calc_shift_operand(op_data_proc_shift_t shift_type, u8 amount, u8 reg_M, bool *carry_out) {
    switch(shift_type) {
    case op_data_proc_shift_LSL:
        return Bit::shift_with_carry<u32>::shift_left(REG(reg_M), amount, carry_out);
    case op_data_proc_shift_LSR:
        return Bit::shift_with_carry<u32>::shift_right(REG(reg_M), amount, carry_out);
    case op_data_proc_shift_ASR:
        return Bit::shift_with_carry<u32>::arithmetic_shift_right(REG(reg_M), amount, carry_out);
    case op_data_proc_shift_ROR:
        return Bit::shift_with_carry<u32>::rotate_right(REG(reg_M), amount, carry_out);
    }
}

void CPU::execute_data_processing_command(CPU::op_data_proc_cmd_t command, bool set_flags, u8 reg_N, u8 reg_D, u32 shift_operand, bool shift_carry_out) {
    switch(command) {
        case op_data_proc_cmd_AND: {
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
            assert(set_flags); //TODO: remove this, just curious :P

            u32 alu_out = REG(reg_N) - shift_operand;

            change_CPSR_N(Bit::test(alu_out, 31));
            change_CPSR_Z(alu_out == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
            break;
        }
        case op_data_proc_cmd_TEQ: {
            assert(set_flags); //TODO: remove this, just curious :P

            u32 alu_out = REG(reg_N) - shift_operand;

            change_CPSR_N(Bit::test(alu_out, 31));
            change_CPSR_Z(alu_out == 0);
            change_CPSR_C(shift_carry_out);
            // V unaffected
            break;
        }
        case op_data_proc_cmd_CMP: {
            assert(set_flags); //TODO: remove this, just curious :P

            u32 alu_out = REG(reg_N) - shift_operand;

            change_CPSR_N(Bit::test(alu_out, 31));
            change_CPSR_Z(alu_out == 0);
            change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) - shift_operand));
            break;
        }
        case op_data_proc_cmd_CMN: {
            assert(set_flags); //TODO: remove this, just curious :P

            u32 alu_out = REG(reg_N) + shift_operand;

            change_CPSR_N(Bit::test(alu_out, 31));
            change_CPSR_Z(alu_out == 0);
            change_CPSR_C(!check_carry_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
            change_CPSR_V(check_overflow_32(REG(reg_N), shift_operand, REG(reg_N) + shift_operand));
            break;
        }
        case op_data_proc_cmd_ORR: {
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

void CPU::data_proc_reg(
    bool condition,
    op_data_proc_cmd_t command,
    bool set_flags,
    u8 reg_N,
    u8 reg_D,
    u8 shift_amount,
    op_data_proc_shift_t shift_type,
    u8 reg_M
) {
    // normal    1  pc+2L  i  0  (pc+2L)  0 1 0
    //              pc+3L
    // dest=pc   1  pc+2L  i  0  (pc+2L)  0 0 0
    //           2  alu    i  0  (alu)    0 1 0
    //           3  alu+L  i  0  (alu+L)  0 1 0
    //              alu+2L

    //cycle 1
    io->tick();
    prefetch();
    if(condition) {
        bool shift_carry_out;
        u32 shift_operand = calc_shift_operand(shift_type, shift_amount, reg_M, &shift_carry_out);
        execute_data_processing_command(
            command,
            set_flags,
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
    bool condition,
    op_data_proc_cmd_t command,
    bool set_flags,
    u8 reg_N,
    u8 reg_D,
    u8 reg_S,
    op_data_proc_shift_t shift_type,
    u8 reg_M
) {
    // shift(Rs) 1  pc+2L  i  0 (pc+2L)   1 0 0
    //           2  pc+3L  i  0 -         0 1 1
    //              pc+3L
    // shift(Rs) 1  pc+8   2  0 (pc+8)    1 0 0
    // dest=pc   2  pc+12  2  0 -         0 0 1
    //           3  alu    2  0 (alu)     0 1 0
    //           4  alu+4  2  0 (alu+4)   0 1 0
    //              alu+8

    //cycle 1
    io->tick();
    prefetch();
    if(condition) {
        u32 shift_amount = REG(reg_S);

        //cycle 2
        io->tick();
        // internal cycle, no prefetch
        bool shift_carry_out;
        u32 shift_operand = calc_shift_operand(shift_type, shift_amount, reg_M, &shift_carry_out);
        execute_data_processing_command(
            command,
            set_flags,
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
    bool condition,
    op_data_proc_cmd_t command,
    bool set_flags,
    u8 reg_N,
    u8 reg_D,
    u8 rotate,
    u8 immediate
) {
    // normal    1  pc+2L  i  0  (pc+2L)  0 1 0
    //              pc+3L
    // dest=pc   1  pc+2L  i  0  (pc+2L)  0 0 0
    //           2  alu    i  0  (alu)    0 1 0
    //           3  alu+L  i  0  (alu+L)  0 1 0
    //              alu+2L

    //cycle 1
    io->tick();
    prefetch();
    if(condition) {
        bool shift_carry_out;
        u32 shift_operand = Bit::shift_with_carry<u32>::rotate_right(immediate, 1 << rotate, &shift_carry_out);
        execute_data_processing_command(
            command,
            set_flags,
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

void CPU::branch(bool condition, bool link, s32 immediate) {
    // 1  pc+2L   i  0  (pc+2L)  0 0 0
    // 2  alu     i  0  (alu)    0 1 0
    // 3  alu+L   i  0  (alu+L)  0 1 0
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