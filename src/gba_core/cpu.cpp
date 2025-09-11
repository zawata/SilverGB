#include "gba_core/cpu.hpp"

#include <bit>
#include <cassert>
#include <nowide/iostream.hpp>
#include <variant>

#include "gba_core/arm_instruction.hh"
#include "gba_core/io.hpp"

#include "util/bit.hpp"
#include "util/flags.hpp"

struct Registers {
    u32 low_regs[8];
    u32 hi_regs[5];
    u32 fiq_hi_regs[5];
    u32 lr[6];
    u32 sp[6];
    u32 pc;
    u32 cpsr;
    u32 spsr[5];
} regs;

#define make_reg_funcs(name, bit) \
    [[maybe_unused]] __force_inline bool get_##name() { return Bit::test(regs.cpsr, bit); } \
    [[maybe_unused]] __force_inline void set_##name() { Bit::set(&regs.cpsr, bit); } \
    [[maybe_unused]] __force_inline void change_##name(bool state) { Bit::set_cond(regs.cpsr, bit, state); } \
    [[maybe_unused]] __force_inline void reset_##name() { Bit::reset(&regs.cpsr, bit); } \
    [[maybe_unused]] __force_inline void flip_##name() { Bit::toggle(regs.cpsr, bit); };

make_reg_funcs(cpsr_N, 31);
make_reg_funcs(cpsr_Z, 30);
make_reg_funcs(cpsr_C, 29);
make_reg_funcs(cpsr_V, 28);

make_reg_funcs(irq_disable, 7);
make_reg_funcs(fiq_disable, 6);
make_reg_funcs(thumb_mode, 5);

__force_inline bool check_carry_16(u16 x, u16 y, u32 r) { return (x ^ y ^ r) & 0x10000; }
__force_inline bool check_carry_16(u16 x, u16 y, u16 z, u32 r) { return (x ^ y ^ z ^ r) & 0x10000; }
__force_inline bool check_overflow_16(s16 x, s16 y, s32 r) { return (x ^ y ^ r) & 0x8000; }
__force_inline bool check_overflow_16(s16 x, s16 y, s16 z, s32 r) { return (x ^ y ^ z ^ r) & 0x8000; }
__force_inline bool check_carry_32(u32 x, u32 y, u64 r) { return (x ^ y ^ r) & 0x100000000; }
__force_inline bool check_carry_32(u32 x, u32 y, u32 z, u64 r) { return (x ^ y ^ z ^ r) & 0x100000000; }
__force_inline bool check_overflow_32(s32 x, s32 y, s64 r) { return (x ^ y ^ r) & 0x80000000; }
__force_inline bool check_overflow_32(s32 x, s32 y, s32 z, s64 r) { return (x ^ y ^ z ^ r) & 0x80000000; }

enum Size : u8 {
    Byte     = 1,
    Halfword = 2,
    Word     = 4,
};

__force_inline u32 getWordSize() { return get_thumb_mode() ? Size::Halfword : Size::Word; }

enum struct OperatingMode {
    FIQ,
    FastInterrupt = FIQ,
    IRQ,
    Interrupt = IRQ,
    SVC,
    Supervisor = SVC,
    ABT,
    Abort = ABT,
    UND,
    Undefined = UND,

    USR,
    User = USR,                // user mode doesn't get an SPSR
    SYS,
    System = SYS,              // system mode uses user-mode registers
} current_mode
        = OperatingMode::User; // for the time being: force user mode

u8 getSPLRIndexForCurrentMode() {
    if(current_mode == OperatingMode::SYS) {
        // system and usr share an SP/LR register bank
        return (u8)OperatingMode::USR;
    } else {
        return (u8)current_mode;
    }
}

u8 getSPSRIndexForCurrentMode() {
    assert(bounded((u8)current_mode, (u8)OperatingMode::FIQ, (u8)OperatingMode::UND));
    return (u8)current_mode;
}

u32 *getReg(u8 reg) {
    if(reg < 8) {
        return &regs.low_regs[reg];
    } else if(reg < 13) {
        if(current_mode == OperatingMode::FIQ) {
            return &regs.fiq_hi_regs[reg - 8];
        } else {
            return &regs.hi_regs[reg - 8];
        }
    } else if(reg == 13) {
        return &regs.lr[getSPLRIndexForCurrentMode()];
    } else if(reg == 14) {
        return &regs.lr[getSPSRIndexForCurrentMode()];
    } else if(reg == 15) {
        return &regs.pc;
    }

    assert(false);
}

u32 *getLR() { return getReg(13); }
u32 *getSP() { return getReg(14); }
u32 *getPC() { return getReg(15); }
u32 *getCPSR() { return &regs.cpsr; }
u32 *getSPSR() { return &regs.spsr[getSPSRIndexForCurrentMode()]; }

CPU::CPU(IO_Bus *io, bool bootrom_enabled) :
    io(io) {
    // fill the instruction pipeline
    prefetch();
    prefetch();
}

CPU::~CPU() { }

void CPU::tick() { execute(); }

bool cond_equal() { return get_cpsr_Z(); }
bool cond_not_equal() { return !cond_equal(); }
bool cond_carry_set() { return get_cpsr_C(); }
bool cond_carry_clear() { return !cond_carry_set(); }
bool cond_minus() { return get_cpsr_N(); }
bool cond_plus() { return !cond_minus(); }
bool cond_overflow() { return get_cpsr_V(); }
bool cond_no_overflow() { return !cond_overflow(); }
bool cond_unsigned_higher() { return cond_carry_set() && cond_not_equal(); }
bool cond_unsigned_lower_or_same() { return !cond_unsigned_higher(); }
bool cond_signed_greater_than_or_equal() { return get_cpsr_N() == get_cpsr_V(); }
bool cond_signed_less_than() { return !cond_signed_greater_than_or_equal(); }
bool cond_signed_greater_than() { return cond_not_equal() && cond_signed_greater_than_or_equal(); }
bool cond_signed_less_than_or_equal() { return !cond_signed_greater_than(); }
bool cond_always() { return true; }
bool cond_never() { return false; }

bool CPU::EvaluateCondition(Arm::Condition condition) {
    static bool (*conditions[])(void) = {
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

    return conditions[(u8)condition]();
}

void CPU::prefetch() {
    op2 = op1;
    op1 = io->read(*getPC());
    *getPC() += getWordSize();
}

void CPU::prefetch(u32 dest) {
    op2 = op1;
    op1 = io->read(dest);
}

// helper type for the visitor
template<class... Ts>
struct overloads: Ts... {
    using Ts::operator()...;
};

void CPU::execute() {
    // we prefetch 2 instructions ahead
    const u32        word  = op2;

    Arm::Instruction instr = Arm::DecodeArm(word);

    std::cout << as_hex(word) << ": " << Arm::DisassembleArm(instr) << std::endl;

    const auto instructionVisitor = overloads {
        [this](Arm::Branch const &b) { branch(b); },
        [this](Arm::BranchAndExchange const &bex) { branch_and_exchange(bex); },
        [this](Arm::DataProcessing const &dp) {
            if(dp.is_imm) {
                data_proc_immediate(dp);
            } else {
                if(dp.ShiftedRegisterOperand.is_reg) {
                    data_proc_shifted_reg(dp);
                } else {
                    data_proc_reg(dp);
                }
            }
        },
        [this](Arm::PSRTransfer const &psr) { psr_transfer(psr); },
        [this](Arm::Multiply const &m) { /*multiply(m);*/ },
        [this](Arm::MultiplyLong const &ml) { /*multiply_long(ml);*/ },
        [this](Arm::SingleDataTransfer const &sdt) {
            if(sdt.is_imm) {
                single_data_transfer_immediate(sdt);
            } else {
                single_data_transfer_shifted_reg(sdt);
            }
        },
        [this](Arm::HalfwordDataTransfer const &hdt) {
            if(hdt.is_imm) {
                halfword_data_transfer_immediate(hdt);
            } else {
                halfword_data_transfer_shifted_reg(hdt);
            }
        },
        [this](Arm::BlockDataTransfer const &bdt) { block_data_transfer(bdt); },
        [this](Arm::Swap const &s) { swap(s); },
        [this](Arm::SoftwareInterrupt const &swi) { std::cout << "Software Interrupt"; },
        [this](Arm::Undefined const &undef) { std::cout << "Undefined Instruction"; },
        [this](Arm::CoprocessorDataOperation const &cdo) { std::cout << "Coprocessor Instructions Not Implemented"; },
        [this](Arm::CoprocessorDataTransfer const &cdt) { std::cout << "Coprocessor Instructions Not Implemented"; },
        [this](Arm::CoprocessorRegisterTransfer const &crt) {
            std::cout << "Coprocessor Instructions Not Implemented";
        }};

    std::visit<void>(instructionVisitor, instr);
}

void throw_data_abort() { nowide::cerr << "data abort" << std::endl; }

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

void CPU::branch(Arm::Branch const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode
    //           1     pc+2L  i  0  pc+2L   N          0
    //           2     alu    i  0  alu     S          0
    //           3     alu+L  i  0  alu+L   S          0
    //                 alu+2L

    // cycle 1
    io->tick();
    prefetch();

    if(EvaluateCondition(i.condition)) {
        // offset is shifted by instruction parser
        u32 ret = *getPC();
        *getPC() += i.offset - 4; // TODO: why -4 ?

        // cycle 2
        io->tick();
        prefetch();
        if(i.link) {
            *getLR() = ret;
        }

        // cycle 3
        io->tick();
        prefetch();
        if(i.link) {
            *getLR() -= 4;
        }
    }
}

// TODO: this function
void CPU::branch_and_exchange(Arm::BranchAndExchange const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode
    //           1     pc+2W  I  0  pc+2W   N          0
    //           2     alu    i  0  alu     S          0
    //           3     alu+w  i  0  alu+W   S          0
    //                 alu+2w

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        // offset is shifted by instruction parser
        *getPC() += *getReg(i.rN);
        nowide::cout << "switching to thumb mode" << std::endl;
        set_thumb_mode();

        // cycle 2
        io->tick();
        prefetch();

        // cycle 3
        io->tick();
        prefetch();
    }
}

void CPU::data_proc_reg(Arm::DataProcessing const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode
    // normal    1     pc+2L  i  0  pc+2L   S          0
    //                 pc+3L
    // dest=pc   1     pc+2L  i  0  pc+2L   N          0
    //           2     alu    i  0  alu     S          0
    //           3     alu+L  i  0  alu+L   S          0
    //                 alu+2L

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        bool shift_carry_out;
        u32  shift_operand = calc_shift_operand(
                i.ShiftedRegisterOperand.shift_type,
                i.ShiftedRegisterOperand.shift_amount,
                i.ShiftedRegisterOperand.rM,
                &shift_carry_out);
        exec_dp_op(i, shift_operand, shift_carry_out);

        if(i.rD == Arm::Registers::PC) {
            // cycle 2
            io->tick();
            prefetch();

            // cycle 3
            io->tick();
            prefetch();
        }
    }
}

void CPU::data_proc_shifted_reg(Arm::DataProcessing const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode
    // shift(Rs) 1     pc+2L  i  0 pc+2L    I          0
    //           2     pc+3L  i  0 -        S          1
    //                 pc+3L
    // shift(Rs) 1     pc+8   2  0 pc+8     I          0
    // dest=pc   2     pc+12  2  0 -        N          1
    //           3     alu    2  0 alu      S          0
    //           4     alu+4  2  0 alu+4    S          0
    //                 alu+8

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        // cycle 2
        io->tick();
        // internal cycle, no prefetch

        bool shift_carry_out;
        u32  shift_operand = calc_shift_operand(
                i.ShiftedRegisterOperand.shift_type,
                *getReg(i.ShiftedRegisterOperand.rS),
                i.ShiftedRegisterOperand.rM,
                &shift_carry_out);
        exec_dp_op(i, shift_operand, shift_carry_out);

        if(i.rD == Arm::Registers::PC) {
            // cycle 3
            io->tick();
            prefetch();

            // cycle 4
            io->tick();
            prefetch();
        }
    }
}

void CPU::data_proc_immediate(Arm::DataProcessing const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode
    // normal    1     pc+2L  i  0  pc+2L   S          0
    //                 pc+3L
    // dest=pc   1     pc+2L  i  0  pc+2L   N          0
    //           2     alu    i  0  alu     S          0
    //           3     alu+L  i  0  alu+L   S          0
    //                 alu+2L

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        bool shift_carry_out;
        u32  shift_operand = Bit::ShiftWithCarry::rotate_right<u32>(
                i.ImmediateOperand.immediate, 1 << i.ImmediateOperand.rotate, &shift_carry_out);
        exec_dp_op(i, shift_operand, shift_carry_out);

        if(i.rD == Arm::Registers::PC) {
            // cycle 2
            io->tick();
            prefetch();

            // cycle 3
            io->tick();
            prefetch();
        }
    }
}

void CPU::psr_transfer(Arm::PSRTransfer const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode
    // normal    1     pc+2L  i  0  pc+2L   S          0
    //                 pc+3L
    // dest=pc   1     pc+2L  i  0  pc+2L   N          0
    //           2     alu    i  0  alu     S          0
    //           3     alu+L  i  0  alu+L   S          0
    //                 alu+2L

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        switch(i.type) {
        case Arm::PSRTransfer::Type::PSRToRegister:
            *getReg(i.PSRToRegister.rD) = *(i.use_spsr ? getSPSR() : getCPSR());
            break;
        case Arm::PSRTransfer::Type::RegisterToPSR:
            *(i.use_spsr ? getSPSR() : getCPSR()) = *getReg(i.PSRToRegister.rD);
            break;
        case Arm::PSRTransfer::Type::RegisterToPSRF:
            u32 operand;
            if(i.RegisterToPSRF.is_imm) {
                operand = Bit::ShiftWithCarry::rotate_right<u32>(
                        i.RegisterToPSRF.ImmediateOperand.value,
                        1 << i.RegisterToPSRF.ImmediateOperand.rotation,
                        nullptr);
            } else {
                operand = *getReg(i.RegisterToPSRF.RegisterOperand.rM);
            }
            *(i.use_spsr ? getSPSR() : getCPSR()) |= (operand & (0xF << 28));
            break;
        default: assert(false);
        }
    }
}

void CPU::single_data_transfer_shifted_reg(Arm::SingleDataTransfer const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode|transfer
    // store
    //           1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  1  Rd      N          1      d
    //                 pc+3L

    // load
    // normal    1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  0  alu     I          1      d
    //           3     pc+3L  i  0  -       S          1      c
    //                 pc+3L
    // dest=pc   1     pc+8   2  0  pc+8    N          0      c
    //           2     alu       0  pc’     I          1      d
    //           3     pc+12  2  0  -       N          1      c
    //           4     pc’    2  0  pc’     S          0      c
    //           5     pc’+4  2  0  pc’+4   S          0      c
    //                 pc’+8

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        u32 shift_operand = calc_shift_operand(
                i.ShiftedRegisterOperand.shift_type,
                i.ShiftedRegisterOperand.shift_amount,
                i.ShiftedRegisterOperand.rM,
                nullptr);

        if(i.load) {
            // cycle 2
            io->tick();
            // internal cycle, no prefetch

            // this will perform cycle 3
            exec_sdt_load_op(i, shift_operand);

            if(i.rD == Arm::Registers::PC) {
                // cycle 4
                io->tick();
                prefetch();

                // cycle 5
                io->tick();
                prefetch();
            }
        } else {
            // cycle 2
            io->tick();
            prefetch();
            exec_sdt_store_op(i, shift_operand);
        }
    }
}

void CPU::single_data_transfer_immediate(Arm::SingleDataTransfer const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode|transfer
    // store
    //           1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  1  Rd      N          1      d
    //                 pc+3L

    // load
    // normal    1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  0  alu     I          1      d
    //           3     pc+3L  i  0  -       S          1      c
    //                 pc+3L
    // dest=pc   1     pc+8   2  0  pc+8    N          0      c
    //           2     alu       0  pc’     I          1      d
    //           3     pc+12  2  0  -       N          1      c
    //           4     pc’    2  0  pc’     S          0      c
    //           5     pc’+4  2  0  pc’+4   S          0      c
    //                 pc’+8

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        if(i.load) {
            // cycle 2
            io->tick();
            // internal cycle, no prefetch

            // this will perform cycle 3
            exec_sdt_load_op(i, i.ImmediateOperand.offset);

            if(i.rD == Arm::Registers::PC) {
                // cycle 4
                io->tick();
                prefetch();

                // cycle 5
                io->tick();
                prefetch();
            }
        } else {
            // cycle 2
            io->tick();
            prefetch();
            exec_sdt_store_op(i, i.ImmediateOperand.offset);
        }
    }
}

void CPU::halfword_data_transfer_shifted_reg(Arm::HalfwordDataTransfer const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode|transfer
    // store
    //           1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  1  Rd      N          1      d
    //                 pc+3L

    // load
    // normal    1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  0  alu     I          1      d
    //           3     pc+3L  i  0  -       S          1      c
    //                 pc+3L
    // dest=pc   1     pc+8   2  0  pc+8    N          0      c
    //           2     alu       0  pc’     I          1      d
    //           3     pc+12  2  0  -       N          1      c
    //           4     pc’    2  0  pc’     S          0      c
    //           5     pc’+4  2  0  pc’+4   S          0      c
    //                 pc’+8

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        if(i.load) {
            // cycle 2
            io->tick();
            // internal cycle, no prefetch

            // this will perform cycle 3
            exec_hwsd_load_op(i, *getReg(i.RegisterOperand.rM));

            if(i.rD == Arm::Registers::PC) {
                // cycle 4
                io->tick();
                prefetch();

                // cycle 5
                io->tick();
                prefetch();
            }
        } else {
            // cycle 2
            io->tick();
            prefetch();
            exec_hwsd_store_op(i, *getReg(i.RegisterOperand.rM));
        }
    }
}

void CPU::halfword_data_transfer_immediate(Arm::HalfwordDataTransfer const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode|transfer
    // store
    //           1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  1  Rd      N          1      d
    //                 pc+3L

    // load
    // normal    1     pc+2L  i  0  pc+2L   N          0      c
    //           2     alu    s  0  alu     I          1      d
    //           3     pc+3L  i  0  -       S          1      c
    //                 pc+3L
    // dest=pc   1     pc+8   2  0  pc+8    N          0      c
    //           2     alu       0  pc’     I          1      d
    //           3     pc+12  2  0  -       N          1      c
    //           4     pc’    2  0  pc’     S          0      c
    //           5     pc’+4  2  0  pc’+4   S          0      c
    //                 pc’+8

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        if(i.load) {
            // cycle 2
            io->tick();
            // internal cycle, no prefetch

            // this will perform cycle 3
            exec_hwsd_load_op(i, i.ImmediateOperand.offset);

            if(i.rD == Arm::Registers::PC) {
                // cycle 4
                io->tick();
                prefetch();

                // cycle 5
                io->tick();
                prefetch();
            }
        } else {
            // cycle 2
            io->tick();
            prefetch();
            exec_hwsd_store_op(i, i.ImmediateOperand.offset);
        }
    }
}

void CPU::block_data_transfer(Arm::BlockDataTransfer const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode|transfer

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        if(i.load) {
            // cycle 2
            io->tick();
            // internal cycle, no prefetch

            bool shift_carry_out;
            // cycle 3-N
            exec_bdt_load_op(i);

            if(Bit::test(i.registers, Arm::Registers::PC)) {
                // cycle N + 1
                io->tick();
                prefetch();

                // cycle N + 2
                io->tick();
                prefetch();
            }
        } else {
            // cycle 2 - N
            io->tick();
            exec_bdt_store_op(i);
        }
    }
}

void CPU::swap(Arm::Swap const &i) {
    //           cycle|addr  |sz|rw|data   |cycle type|opcode|transfer
    // normal    1     pc+8   i  0  pc+2L   N          0      c
    //           2     Rn     s  0  alu     I          1      d
    //           3     Rn     i  0  -       S          1      c
    //           4     pc+12
    //                 pc+12

    // cycle 1
    io->tick();
    prefetch();
    if(EvaluateCondition(i.condition)) {
        // cycle 2
        io->tick();
        // internal cycle, no prefetch

        if(i.rD == Arm::Registers::PC) {
            // cycle 4
            io->tick();
            prefetch();

            // cycle 5
            io->tick();
            prefetch();
        }
    }
}

inline u32 CPU::calc_shift_operand(Arm::ShiftType shift_type, u8 amount, u8 reg_M, bool *carry_out) {
    switch(shift_type) {
    case Arm::ShiftType::LogicalLeft:
        return Bit::ShiftWithCarry::logical_left<u32>(*getReg(reg_M), amount, get_cpsr_C(), carry_out);
    case Arm::ShiftType::LogicalRight:
        return Bit::ShiftWithCarry::logical_right<u32>(*getReg(reg_M), amount, carry_out);
    case Arm::ShiftType::ArithmeticRight:
        return Bit::ShiftWithCarry::arithmetic_right<u32>(*getReg(reg_M), amount, carry_out);
    case Arm::ShiftType::RotateRight: return Bit::ShiftWithCarry::rotate_right<u32>(*getReg(reg_M), amount, carry_out);
    default:                          assert(false);
    }
}

inline void CPU::exec_dp_op(Arm::DataProcessing const &i, u32 shift_operand, bool shift_carry_out) {
    using OpCode = Arm::DataProcessing::OperationCode;

    switch(i.operation_code) {
    case OpCode::AND: {
        *getReg(i.rD) = *getReg(i.rN) & shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::EOR: {
        *getReg(i.rD) = *getReg(i.rN) ^ shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::SUB: {
        *getReg(i.rD) = *getReg(i.rN) - shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(!check_carry_32(*getReg(i.rN), shift_operand, *getReg(i.rN) - shift_operand));
            change_cpsr_V(check_overflow_32(*getReg(i.rN), shift_operand, *getReg(i.rN) - shift_operand));
        }
        break;
    }
    case OpCode::RSB: {
        *getReg(i.rD) = shift_operand - *getReg(i.rN);

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(!check_carry_32(*getReg(i.rN), shift_operand, shift_operand - *getReg(i.rN)));
            change_cpsr_V(check_overflow_32(*getReg(i.rN), shift_operand, shift_operand - *getReg(i.rN)));
        }
        break;
    }
    case OpCode::ADD: {
        *getReg(i.rD) = *getReg(i.rN) + shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(check_carry_32(*getReg(i.rN), shift_operand, *getReg(i.rN) + shift_operand));
            change_cpsr_V(check_overflow_32(*getReg(i.rN), shift_operand, *getReg(i.rN) + shift_operand));
        }
        break;
    }
    case OpCode::ADC: {
        *getReg(i.rD) = *getReg(i.rN) + shift_operand + (get_cpsr_C() ? 1 : 0);

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(check_carry_32(*getReg(i.rN), shift_operand, *getReg(i.rN) + shift_operand));
            change_cpsr_V(check_overflow_32(*getReg(i.rN), shift_operand, *getReg(i.rN) + shift_operand));
        }
        break;
    }
    case OpCode::SBC: {
        *getReg(i.rD) = *getReg(i.rN) - shift_operand - (!get_cpsr_C() ? 1 : 0);

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(!check_carry_32(
                    *getReg(i.rN), shift_operand, *getReg(i.rN) - shift_operand - (!get_cpsr_C() ? 1 : 0)));
            change_cpsr_V(check_overflow_32(
                    *getReg(i.rN), shift_operand, *getReg(i.rN) - shift_operand - (!get_cpsr_C() ? 1 : 0)));
        }
        break;
    }
    case OpCode::RSC: {
        *getReg(i.rD) = shift_operand - *getReg(i.rN) - (!get_cpsr_C() ? 1 : 0);

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(!check_carry_32(
                    *getReg(i.rN), shift_operand, shift_operand - *getReg(i.rN) - (!get_cpsr_C() ? 1 : 0)));
            change_cpsr_V(check_overflow_32(
                    *getReg(i.rN), shift_operand, shift_operand - *getReg(i.rN) - (!get_cpsr_C() ? 1 : 0)));
        }
        break;
    }
    case OpCode::TST: {
        u32 alu_out = *getReg(i.rN) - shift_operand;

        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(shift_carry_out);
        // V unaffected
        break;
    }
    case OpCode::TEQ: {
        u32 alu_out = *getReg(i.rN) - shift_operand;

        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(shift_carry_out);
        // V unaffected
        break;
    }
    case OpCode::CMP: {
        u32 alu_out = *getReg(i.rN) - shift_operand;
        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(!check_carry_32(*getReg(i.rN), shift_operand, *getReg(i.rN) - shift_operand));
        change_cpsr_V(check_overflow_32(*getReg(i.rN), shift_operand, *getReg(i.rN) - shift_operand));
        break;
    }
    case OpCode::CMN: {
        u32 alu_out = *getReg(i.rN) + shift_operand;

        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(!check_carry_32(*getReg(i.rN), shift_operand, *getReg(i.rN) + shift_operand));
        change_cpsr_V(check_overflow_32(*getReg(i.rN), shift_operand, *getReg(i.rN) + shift_operand));
        break;
    }
    case OpCode::ORR: {
        *getReg(i.rD) = *getReg(i.rN) | shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::MOV: {
        *getReg(i.rN) = shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rN), 31));
            change_cpsr_Z(*getReg(i.rN) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::BIC: {
        *getReg(i.rD) = *getReg(i.rN) & ~shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rD), 31));
            change_cpsr_Z(*getReg(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::MVN: {
        *getReg(i.rN) = ~shift_operand;

        if(i.set_flags) {
            change_cpsr_N(Bit::test(*getReg(i.rN), 31));
            change_cpsr_Z(*getReg(i.rN) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    }
}

inline void CPU::exec_sdt_load_op(Arm::SingleDataTransfer const &i, u32 offset) {
#define calc_sub_idx(reg, offset)          (*getReg(i.rN) - offset)
#define calc_add_idx(reg, offset)          (*getReg(i.rN) + offset)
#define calc_auto_idx(reg, offset, is_add) (is_add ? calc_add_idx(reg, offset) : calc_sub_idx(reg, offset))

    u32 index;
    if(i.is_pre_idx) {
        index = calc_auto_idx(i.rN, offset, i.is_inc);
        if(!i.is_byte && (index % getWordSize()) != 0) {
            throw_data_abort();
        }
        if(i.write_back) {
            *getReg(i.rN) = index;
        }
    } else {
        index = *getReg(i.rN);
    }

    // cycle 3
    io->tick();
    prefetch();
    *getReg(i.rD) = io->read(index) & (!i.is_byte ? 0xFFFFFFFF : 0xFF);

    if(!i.is_pre_idx) {
        index         = calc_auto_idx(i.rN, offset, i.is_inc);

        // base reg is always written on a post-idx
        *getReg(i.rN) = index;
    }

#undef calc_auto_idx
#undef calc_add_idx
#undef calc_sub_idx
}

inline void CPU::exec_sdt_store_op(Arm::SingleDataTransfer const &i, u32 offset) {
#define calc_sub_idx(reg, offset)          (*getReg(reg) - (offset))
#define calc_add_idx(reg, offset)          (*getReg(reg) + (offset))
#define calc_auto_idx(reg, offset, is_add) ((is_add) ? calc_add_idx(reg, offset) : calc_sub_idx(reg, offset))

    u32 index;
    if(i.is_pre_idx) {
        index = calc_auto_idx(i.rN, offset, i.is_inc);
        if(!i.is_byte && (index % getWordSize()) != 0) {
            throw_data_abort();
        }
        if(i.write_back) {
            *getReg(i.rN) = index;
        }
    } else {
        index = *getReg(i.rN);
    }

    io->write(index, *getReg(i.rD));

    if(!i.is_pre_idx) {
        index         = calc_auto_idx(i.rN, offset, i.is_inc);

        // base reg is always written on a post-idx
        *getReg(i.rN) = index;
    }

#undef calc_auto_idx
#undef calc_add_idx
#undef calc_sub_idx
}

inline u32 compute_hwsd_result(u32 data, bool halfwords, bool signed_data) {
    if(!halfwords) {
        bool sign     = Bit::test(data, 7);
        u32  signMask = sign && signed_data ? 0xFFFFFF00 : 0;
        return signMask | (data & 0xFF);
    } else {
        bool sign     = Bit::test(data, 15);
        u32  signMask = sign ? 0xFFFF0000 : 0;
        return signMask | (data & 0xFFFF);
    }
}

inline void CPU::exec_hwsd_load_op(Arm::HalfwordDataTransfer const &i, u32 offset) {
#define calc_sub_idx(reg, offset)          (*getReg(reg) - (offset))
#define calc_add_idx(reg, offset)          (*getReg(reg) + (offset))
#define calc_auto_idx(reg, offset, is_add) ((is_add) ? calc_add_idx(reg, offset) : calc_sub_idx(reg, offset))

    u32 index;
    if(i.is_pre_idx) {
        index = calc_auto_idx(i.rN, offset, i.is_inc);
        if(!i.halfwords && (index % 2) != 0) {
            throw_data_abort(); // TODO: unpredictable behavior
        }
        if(i.write_back) {
            *getReg(i.rN) = index;
        }
    } else {
        index = *getReg(i.rN);
    }

    // cycle 3
    io->tick();
    prefetch();

    *getReg(i.rD) = compute_hwsd_result(io->read(index), i.halfwords, i.signed_data);

    if(!i.is_pre_idx) {
        index         = calc_auto_idx(i.rN, offset, i.is_inc);

        // base reg is always written on a post-idx?
        *getReg(i.rN) = index;
    }

#undef calc_auto_idx
#undef calc_add_idx
#undef calc_sub_idx
}

inline void CPU::exec_hwsd_store_op(Arm::HalfwordDataTransfer const &i, u32 offset) {
#define calc_sub_idx(reg, offset)          (*getReg(reg) - (offset))
#define calc_add_idx(reg, offset)          (*getReg(reg) + (offset))
#define calc_auto_idx(reg, offset, is_add) ((is_add) ? calc_add_idx(reg, offset) : calc_sub_idx(reg, offset))

    u32 index;
    if(i.is_pre_idx) {
        index = calc_auto_idx(i.rN, offset, i.is_inc);
        if(!i.halfwords && (index % 2) != 0) {
            throw_data_abort(); // TODO: unpredictable behavior
        }
        if(i.write_back) {
            *getReg(i.rN) = index;
        }
    } else {
        index = *getReg(i.rN);
    }

    // cycle 3
    io->tick();
    prefetch();

    io->write(index, compute_hwsd_result(*getReg(i.rD), i.halfwords, i.signed_data));

    if(!i.is_pre_idx) {
        index         = calc_auto_idx(i.rN, offset, i.is_inc);

        // base reg is always written on a post-idx?
        *getReg(i.rN) = index;
    }

#undef calc_auto_idx
#undef calc_add_idx
#undef calc_sub_idx
}

inline void CPU::exec_bdt_store_op(Arm::BlockDataTransfer const &i) {
    u8  reg_count = std::popcount(i.registers);

    u32 start_index;
    u32 index;

    if(i.is_inc) {
        index       = *getReg(i.rN) + (reg_count << 2);
        start_index = *getReg(i.rN);
        if(i.is_pre_idx) {
            start_index += getWordSize();
        }
    } else {
        index       = *getReg(i.rN) - (reg_count << 2);
        start_index = index;
        if(!i.is_pre_idx) {
            start_index += getWordSize();
        }
    }

    if((start_index % getWordSize()) != 0) {
        throw_data_abort();
    }

    if(i.write_back) {
        *getReg(i.rN) = index;
    }

    u16 registers = i.registers;
    for(u8 j = 0; j < 16; j++) {
        if(!Bit::test(registers, j)) {
            continue;
        }

        if(j == 15 && i.load_psr) {
            *getCPSR() = *getSPSR();
        }

        io->tick();
        prefetch();

        io->write(start_index + (j << 2), *getReg(j));
    }
}

inline void CPU::exec_bdt_load_op(Arm::BlockDataTransfer const &i) {
    u8  reg_count = std::popcount(i.registers);

    u32 start_index;
    u32 index;

    if(i.is_inc) {
        index       = *getReg(i.rN) + (reg_count << 2);
        start_index = *getReg(i.rN);
        if(i.is_pre_idx) {
            start_index += getWordSize();
        }
    } else {
        index       = *getReg(i.rN) - (reg_count << 2);
        start_index = index;
        if(!i.is_pre_idx) {
            start_index += getWordSize();
        }
    }

    if((start_index % getWordSize()) != 0) {
        throw_data_abort();
    }

    if(i.write_back) {
        *getReg(i.rN) = index;
    }

    u16 registers = i.registers;
    for(u8 j = 0; j < 16; j++) {
        if(!Bit::test(registers, j)) {
            continue;
        }

        if(j == 15 && i.load_psr) {
            *getCPSR() = *getSPSR();
        }

        io->tick();
        prefetch();

        *getReg(j) = io->read(start_index + (j << 2));
    }
}
