#include <cassert>
#include <functional>

#include <nowide/iostream.hpp>

#include "gba_core/cpu.hpp"
#include "gba_core/arm_instruction.hh"

#include "gba_core/io.hpp"
#include "util/bit.hpp"
#include "util/file.hpp"
#include "util/flags.hpp"

u32 gen_regs[16];
u32 psr_regs[6]; // CPSR + SPSR*5

#define REG(X) (gen_regs[(X)])
#define PSR(X) (psr_regs[(X)])

#define LR 14
#define PC 15

#define REG_LR (REG(LR))
#define REG_PC (REG(PC))

#define CPSR 0

#define REG_CPSR (REG(CPSR))

#define BOOL(x) (!(!(x)))
#define BitTest(arg,posn)       BOOL((arg) & (1L << (posn)))
#define BitSet(arg,posn)        ((arg) | (1L << (posn)))
#define BitChange(arg,posn,stt) (((arg) & ~(1UL << (posn))) | ((stt) << (posn)))
#define BitReset(arg,posn)      ((arg) & ~(1L << (posn)))
#define BitFlip(arg,posn)       ((arg) ^ (1L << (posn)))

#define make_reg_funcs(name, bit) \
  __force_inline bool get_##name()              { return BitTest(REG_CPSR, bit); } \
  __force_inline void set_##name()              { REG_CPSR = BitSet(REG_CPSR, bit); } \
  __force_inline void change_##name(bool state) { REG_CPSR = BitChange(REG_CPSR, bit, (u8)state);} \
  __force_inline void reset_##name()            { REG_CPSR = BitReset(REG_CPSR, bit); } \
  __force_inline void flip_##name()             { REG_CPSR = BitFlip(REG_CPSR, bit); };

make_reg_funcs(cpsr_N, 31);
make_reg_funcs(cpsr_Z, 30);
make_reg_funcs(cpsr_C, 29);
make_reg_funcs(cpsr_V, 28);

make_reg_funcs(irq_disable, 7);
make_reg_funcs(fiq_disable, 6);
make_reg_funcs(thumb_mode,  5);

__force_inline bool    check_carry_16(u16 x, u16 y,        u32 r) { return (x^y^r)   & 0x10000; }
__force_inline bool    check_carry_16(u16 x, u16 y, u16 z, u32 r) { return (x^y^z^r) & 0x10000; }
__force_inline bool check_overflow_16(s16 x, s16 y,        s32 r) { return (x^y^r)   & 0x8000; }
__force_inline bool check_overflow_16(s16 x, s16 y, s16 z, s32 r) { return (x^y^z^r) & 0x8000; }
__force_inline bool    check_carry_32(u32 x, u32 y,        u64 r) { return (x^y^r)   & 0x100000000; }
__force_inline bool    check_carry_32(u32 x, u32 y, u32 z, u64 r) { return (x^y^z^r) & 0x100000000; }
__force_inline bool check_overflow_32(s32 x, s32 y,        s64 r) { return (x^y^r)   & 0x80000000; }
__force_inline bool check_overflow_32(s32 x, s32 y, s32 z, s64 r) { return (x^y^z^r) & 0x80000000; }

enum struct OperatingMode {
  FIQ, FastInterrupt = FIQ,
  IRQ, Interrupt = IRQ,
  SVC, Supervisor = SVC,
  ABT, Abort = ABT,
  UND, Undefined = UND,

  USR, User = USR,   // user mode doesn't get an SPSR
  SYS, System = SYS, // system mode uses user-mode registers
} current_mode = OperatingMode::User; // for the time being: force user mode

u8 getSPSRIndexForCurrentMode() {
  assert(bounded((u8)current_mode, (u8)OperatingMode::FIQ, (u8)OperatingMode::UND));
  return (u8)current_mode;
}

CPU::CPU(IO_Bus *io, bool bootrom_enabled) :
io(io) {
    REG_PC = 0;
    // fill the instruction pipeline
    prefetch32();
    prefetch32(REG_PC + 4);
}

CPU::~CPU() {}

void CPU::tick() {
    // TODO: we need to be careful about keeping the instruction pipeline full
     if(PC <= 4) {
       prefetch32();
       return;
     }
    nowide::cout << "PC: " << REG_PC << std::endl;
    execute();
}

bool cond_equal()                        { return get_cpsr_Z(); }
bool cond_not_equal()                    { return !cond_equal(); }
bool cond_carry_set()                    { return get_cpsr_C(); }
bool cond_carry_clear()                  { return !cond_carry_set(); }
bool cond_minus()                        { return get_cpsr_N(); }
bool cond_plus()                         { return !cond_minus(); }
bool cond_overflow()                     { return get_cpsr_V(); }
bool cond_no_overflow()                  { return !cond_overflow(); }
bool cond_unsigned_higher()              { return cond_carry_set() && cond_not_equal(); }
bool cond_unsigned_lower_or_same()       { return !cond_unsigned_higher(); }
bool cond_signed_greater_than_or_equal() { return get_cpsr_N() == get_cpsr_V(); }
bool cond_signed_less_than()             { return !cond_signed_greater_than_or_equal(); }
bool cond_signed_greater_than()          { return cond_not_equal() && cond_signed_greater_than_or_equal(); }
bool cond_signed_less_than_or_equal()    { return !cond_signed_greater_than(); }
bool cond_always()                       { return true; }
bool cond_never()                        { return false; }

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

void CPU::prefetch16() {
    nowide::cout << "prefetch16() " << as_hex(REG_PC) << std::endl;
    op2 = op1;
    op1 = io->read(REG_PC);
}

void CPU::prefetch16(u16 dest) {
    nowide::cout << "prefetch16(u16) " << as_hex(REG_PC) << std::endl;
    op2 = op1;
    op1 = io->read(dest);
}

void CPU::prefetch32() {
    nowide::cout << "prefetch32() " << as_hex(REG_PC) << std::endl;
    op2 = op1;
    op1 = io->read(REG_PC);
}

void CPU::prefetch32(u32 dest) {
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

   Arm::Instruction instr = Arm::Instruction::Decode(word);

    using Instr = Arm::InstructionType;
    std::cout << instr.Disassemble() << std::endl;
    switch(instr.Type()) {
    case Instr::Branch: {
       branch(instr);
       break;
    }
    case Instr::BranchAndExchange: {
       branch_and_exchange(instr);
       break;
    }
    case Instr::DataProcessing: {
       std::cout << instr.Disassemble();
       auto &i = instr.InstructionData<Arm::DataProcessing>();
       if(i.is_imm) {
         data_proc_immediate(instr);
       } else {
         if(i.ShiftedRegisterOperand.is_reg) {
           data_proc_shifted_reg(instr);
         } else {
           data_proc_reg(instr);
         }
       }
       break;
    }
    case Instr::PSRTransfer: {
       auto &i = instr.InstructionData<Arm::PSRTransfer>();
       psr_transfer(instr);
       break;
    }
    case Instr::Multiply: {
//       multiply(instr);
       break;
    }
    case Instr::MultiplyLong: {
//       multiply_long(instr);
       break;
    }
    case Instr::SingleDataTransfer: {
//       multiply_long(instr);
       break;
    }
    case Instr::HalfwordDataTransfer: {
       break;
    }
    case Instr::BlockDataTransfer: {
       break;
    }
    case Instr::Swap: {
       break;
    }
    case Instr::SoftwareInterrupt:
    case Instr::Undefined: {
       std::cout << instr.Disassemble();
    }
    case Instr::CoprocessorDataOperation:
    case Instr::CoprocessorDataTransfer:
    case Instr::CoprocessorRegisterTransfer: {
       std::cout << instr.Disassemble();
       break;
    }
    default:
        nowide::cout << "error";
    }
}

void throw_data_abort() {
    nowide::cerr << "data abort" << std::endl;
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

void CPU::branch(Arm::Instruction instr) {
    // 2S + 1N
    // 1  pc+2L   i  0  (pc+2L)  N 0
    // 2  alu     i  0  (alu)    S 0
    // 3  alu+L   i  0  (alu+L)  S 0
    //    alu+2L

    auto &i = instr.InstructionData<Arm::Branch>();

    //cycle 1
    io->tick();
    prefetch32();

    if(EvaluateCondition(i.condition)) {
        //TODO: not exactly sure where PC gets set. its either:
        // - in cycle 1 so the LR register is updated with a temp former PC value in cycle 2
        // - in cycle 2 so the second 2 prefetches are performed from PC
        // - in cycle 3 so the prefetches are computed from the alu result
        // the last one seems cleanest imo

        // offset is shifted by instruction parser
        u32 dest = REG_PC + i.offset;

        //cycle 2
        io->tick();
        prefetch32(dest);
        if(i.link) {
          REG_LR = REG_PC;
        }

        //cycle 3
        io->tick();
        prefetch32(dest + 4);
        if(i.link) {
          REG_LR -= 4;
        }
        REG_PC = dest << 2;
    }
}

//TODO: this function
void CPU::branch_and_exchange(Arm::Instruction instr) {
    // 1  pc+2W   I  0  (pc+2W)  N 0
    // 2  alu     i  0  (alu)    S 0
    // 3  alu+w   i  0  (alu+W)  S 0
    //    alu+2w

    auto &i = instr.InstructionData<Arm::BranchAndExchange>();

    //cycle 1
    io->tick();
    prefetch32();
    if(EvaluateCondition(i.condition)) {
        //TODO: not exactly sure where PC gets set. its either:
        // - in cycle 1 so the LR register is updated with a temp former PC value in cycle 2
        // - in cycle 2 so the second 2 prefetches are performed from PC
        // - in cycle 3 so the prefetches are computed from the alu result
        // the last one seems cleanest imo

        // offset is shifted by instruction parser
        u32 dest = REG_PC + REG(i.rN);
        nowide::cout << "switching to thumb mode" << std::endl;
        set_thumb_mode();

        //cycle 2
        io->tick();
        prefetch16(dest);

        //cycle 3
        io->tick();
        prefetch16(dest + 4);
        REG_PC = dest << 2;
    }
}

void CPU::data_proc_reg(Arm::Instruction instr) {
    // normal    1  pc+2L  i  0  (pc+2L)  S 0
    //              pc+3L
    // dest=pc   1  pc+2L  i  0  (pc+2L)  N 0
    //           2  alu    i  0  (alu)    S 0
    //           3  alu+L  i  0  (alu+L)  S 0
    //              alu+2L

    auto &i = instr.InstructionData<Arm::DataProcessing>();

    //cycle 1
    io->tick();
    prefetch32();
    if(EvaluateCondition(i.condition)) {
        bool shift_carry_out;
        u32 shift_operand = calc_shift_operand(
            i.ShiftedRegisterOperand.shift_type,
            i.ShiftedRegisterOperand.shift_amount,
            i.ShiftedRegisterOperand.rM,
            &shift_carry_out
        );
        exec_dp_op(instr, shift_operand, shift_carry_out);

        if(i.rD == PC) {
            //cycle 2
            io->tick();
            prefetch32();

            //cycle 3
            io->tick();
            prefetch32();
        }
    }
}

void CPU::data_proc_shifted_reg(Arm::Instruction instr) {
    // shift(Rs) 1  pc+2L  i  0 (pc+2L)   I 0
    //           2  pc+3L  i  0 -         S 1
    //              pc+3L
    // shift(Rs) 1  pc+8   2  0 (pc+8)    I 0
    // dest=pc   2  pc+12  2  0 -         N 1
    //           3  alu    2  0 (alu)     S 0
    //           4  alu+4  2  0 (alu+4)   S 0
    //              alu+8

    auto &i = instr.InstructionData<Arm::DataProcessing>();

    //cycle 1
    io->tick();
    prefetch32();
    if(EvaluateCondition(i.condition)) {
        //cycle 2
        io->tick();
        // internal cycle, no prefetch

        bool shift_carry_out;
        u32 shift_operand = calc_shift_operand(
            i.ShiftedRegisterOperand.shift_type,
            REG(i.ShiftedRegisterOperand.rS),
            i.ShiftedRegisterOperand.rM,
            &shift_carry_out
        );
        exec_dp_op(instr, shift_operand, shift_carry_out);

        if(i.rD == PC) {
            //cycle 3
            io->tick();
            prefetch32();

            //cycle 4
            io->tick();
            prefetch32();
        }
    }
}

void CPU::data_proc_immediate(Arm::Instruction instr) {
    // normal    1  pc+2L  i  0  (pc+2L)  S 0
    //              pc+3L
    // dest=pc   1  pc+2L  i  0  (pc+2L)  N 0
    //           2  alu    i  0  (alu)    S 0
    //           3  alu+L  i  0  (alu+L)  S 0
    //              alu+2L

    auto &i = instr.InstructionData<Arm::DataProcessing>();

    //cycle 1
    io->tick();
    prefetch32();
    if(EvaluateCondition(i.condition)) {
        bool shift_carry_out;
        u32 shift_operand = Bit::shift_with_carry<u32>::rotate_right(i.ImmediateOperand.immediate, 1 << i.ImmediateOperand.rotate, &shift_carry_out);
        exec_dp_op(instr, shift_operand, shift_carry_out);

        if(i.rD == PC) {
            //cycle 2
            io->tick();
            prefetch32();

            //cycle 3
            io->tick();
            prefetch32();
        }
    }
}

void CPU::psr_transfer(Arm::Instruction instr) {
    // normal    1  pc+2L  i  0  (pc+2L)  S 0
    //              pc+3L
    // dest=pc   1  pc+2L  i  0  (pc+2L)  N 0
    //           2  alu    i  0  (alu)    S 0
    //           3  alu+L  i  0  (alu+L)  S 0
    //              alu+2L

    auto &i = instr.InstructionData<Arm::PSRTransfer>();

    //cycle 1
    io->tick();
    prefetch32();
    if(EvaluateCondition(i.condition)) {
        switch (i.type) {
        case Arm::PSRTransfer::Type::PSRToRegister:
          REG(i.PSRToRegister.rD) = PSR(getSPSRIndexForCurrentMode());
          break;
        case Arm::PSRTransfer::Type::RegisterToPSR:
          PSR(getSPSRIndexForCurrentMode()) = REG(i.PSRToRegister.rD);
          break;
        case Arm::PSRTransfer::Type::RegisterToPSRF:
          u32 operand;
          if(i.RegisterToPSRF.is_imm) {
           bool shift_carry_out;
           operand = Bit::shift_with_carry<u32>::rotate_right(
               i.RegisterToPSRF.ImmediateOperand.value,
               1 << i.RegisterToPSRF.ImmediateOperand.rotation,
               &shift_carry_out);
          } else {
           operand = REG(i.RegisterToPSRF.RegisterOperand.rM);
          }
          PSR(getSPSRIndexForCurrentMode()) |= (operand & (0xF << 28));
          break;
        default:
            assert(false);
        }
    }
}



void CPU::load_store_shifted_reg(Arm::Instruction instr) {
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

    auto &i = instr.InstructionData<Arm::SingleDataTransfer>();

    //cycle 1
    io->tick();
    prefetch32();
    if(EvaluateCondition(i.condition)) {

        //cycle 2
        io->tick();
        // internal cycle, no prefetch

        //this will perform cycle 3
//        execute_load_store_command(
//            true,
//            pre_index,
//            add,
//            word,
//            write,
//            reg_N,
//            reg_D,
//            calc_shift_operand(shift_type, shift_amount, reg_M, nullptr));

        if(i.rD == PC) {
            //cycle 4
            io->tick();
            prefetch32();

            //cycle 5
            io->tick();
            prefetch32();
        }
    }
}

void CPU::load_store_immediate(Arm::Instruction instr) {
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

    auto &i = instr.InstructionData<Arm::SingleDataTransfer>();

    //cycle 1
    io->tick();
    prefetch32();
    if(EvaluateCondition(i.condition)) {

        //cycle 2
        io->tick();
        // internal cycle, no prefetch

        //this will perform cycle 3
//        execute_load_store_command(
//            true,
//            pre_index,
//            add,
//            word,
//            write,
//            reg_N,
//            reg_D,
//            immediate);

        if(i.rD == PC) {
            //cycle 4
            io->tick();
            prefetch32();

            //cycle 5
            io->tick();
            prefetch32();
        }
    }
}

inline u32 CPU::calc_shift_operand(Arm::ShiftType shift_type, u8 amount, u8 reg_M, bool *carry_out) {
    switch(shift_type) {
    case Arm::ShiftType::LogicalLeft:
        return Bit::shift_with_carry<u32>::shift_left(REG(reg_M), amount, get_cpsr_C(), carry_out);
    case Arm::ShiftType::LogicalRight:
        return Bit::shift_with_carry<u32>::shift_right(REG(reg_M), amount, carry_out);
    case Arm::ShiftType::ArithmeticRight:
        return Bit::shift_with_carry<u32>::arithmetic_shift_right(REG(reg_M), amount, carry_out);
    case Arm::ShiftType::RotateRight:
        return Bit::shift_with_carry<u32>::rotate_right(REG(reg_M), amount, carry_out);
    default:
        assert(false);
    }
}

inline void CPU::exec_dp_op(Arm::Instruction instr, u32 shift_operand, bool shift_carry_out) {
    using OpCode = Arm::DataProcessing::OperationCode;

    auto &i = instr.InstructionData<Arm::DataProcessing>();

    bool set_flags = Bit::test((u64)i.operation_code, 0);
    switch((OpCode)((u32)i.operation_code >> 1)) {
    case OpCode::AND: {
        nowide::cout << "AND" << std::endl;
        REG(i.rD) = REG(i.rN) & shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::EOR: {
        nowide::cout << "EOR" << std::endl;
        REG(i.rD) = REG(i.rN) ^ shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::SUB: {
        nowide::cout << "SUB" << std::endl;
        REG(i.rD) = REG(i.rN) - shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(!check_carry_32(REG(i.rN), shift_operand, REG(i.rN) - shift_operand));
            change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, REG(i.rN) - shift_operand));
        }
        break;
    }
    case OpCode::RSB: {
        nowide::cout << "RSB" << std::endl;
        REG(i.rD) = shift_operand - REG(i.rN);

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(!check_carry_32(REG(i.rN), shift_operand, shift_operand - REG(i.rN)));
            change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, shift_operand - REG(i.rN)));
        }
        break;
    }
    case OpCode::ADD: {
        nowide::cout << "ADD" << std::endl;
        REG(i.rD) = REG(i.rN) + shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(check_carry_32(REG(i.rN), shift_operand, REG(i.rN) + shift_operand));
            change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, REG(i.rN) + shift_operand));
        }
        break;
    }
    case OpCode::ADC: {
        nowide::cout << "ADC" << std::endl;
        REG(i.rD) = REG(i.rN) + shift_operand + (get_cpsr_C() ? 1 : 0);

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(check_carry_32(REG(i.rN), shift_operand, REG(i.rN) + shift_operand));
            change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, REG(i.rN) + shift_operand));
        }
        break;
    }
    case OpCode::SBC: {
        nowide::cout << "SBC" << std::endl;
        REG(i.rD) = REG(i.rN) - shift_operand - (!get_cpsr_C() ? 1 : 0);

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(!check_carry_32(REG(i.rN), shift_operand, REG(i.rN) - shift_operand - (!get_cpsr_C() ? 1 : 0)));
            change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, REG(i.rN) - shift_operand - (!get_cpsr_C() ? 1 : 0)));
        }
        break;
    }
    case OpCode::RSC: {
        nowide::cout << "RSC" << std::endl;
        REG(i.rD) = shift_operand - REG(i.rN) - (!get_cpsr_C() ? 1 : 0);

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(!check_carry_32(REG(i.rN), shift_operand, shift_operand - REG(i.rN) - (!get_cpsr_C() ? 1 : 0)));
            change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, shift_operand - REG(i.rN) - (!get_cpsr_C() ? 1 : 0)));
        }
        break;
    }
    case OpCode::TST: {
        nowide::cout << "TST" << std::endl;
        u32 alu_out = REG(i.rN) - shift_operand;

        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(shift_carry_out);
        // V unaffected
        break;
    }
    case OpCode::TEQ: {
        nowide::cout << "TEQ" << std::endl;
        u32 alu_out = REG(i.rN) - shift_operand;

        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(shift_carry_out);
        // V unaffected
        break;
    }
    case OpCode::CMP: {
        nowide::cout << "CMP" << std::endl;
        u32 alu_out = REG(i.rN) - shift_operand;
        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(!check_carry_32(REG(i.rN), shift_operand, REG(i.rN) - shift_operand));
        change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, REG(i.rN) - shift_operand));
        break;
    }
    case OpCode::CMN: {
        nowide::cout << "CMN" << std::endl;
        u32 alu_out = REG(i.rN) + shift_operand;

        change_cpsr_N(Bit::test(alu_out, 31));
        change_cpsr_Z(alu_out == 0);
        change_cpsr_C(!check_carry_32(REG(i.rN), shift_operand, REG(i.rN) + shift_operand));
        change_cpsr_V(check_overflow_32(REG(i.rN), shift_operand, REG(i.rN) + shift_operand));
        break;
    }
    case OpCode::ORR: {
        nowide::cout << "ORR" << std::endl;
        REG(i.rD) = REG(i.rN) | shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::MOV: {
        nowide::cout << "MOV" << std::endl;
        REG(i.rN) = shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rN), 31));
            change_cpsr_Z(REG(i.rN) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::BIC: {
        nowide::cout << "BIC" << std::endl;
        REG(i.rD) = REG(i.rN) & ~shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rD), 31));
            change_cpsr_Z(REG(i.rD) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    case OpCode::MVN: {
        nowide::cout << "MVN" << std::endl;
        REG(i.rN) = ~shift_operand;

        if(set_flags) {
            change_cpsr_N(Bit::test(REG(i.rN), 31));
            change_cpsr_Z(REG(i.rN) == 0);
            change_cpsr_C(shift_carry_out);
            // V unaffected
        }
        break;
    }
    }
}

inline void CPU::exec_ls_op(
  bool load,
  bool pre_index,
  bool add,
  bool word,
  bool write,
  u8 reg_N,
  u8 reg_D,
  u32 offset
) {
#define calc_sub_idx(reg, offset) (REG(reg_N) - offset)
#define calc_add_idx(reg, offset) (REG(reg_N) + offset)
#define calc_auto_idx(reg, offset, is_add) (is_add ? calc_add_idx(reg, offset) : calc_sub_idx(reg, offset))

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
    prefetch32();
    REG(reg_D) = io->read(index) & (word ? 0xFFFFFFFF : 0xFF);

    if(!pre_index) {
        index = calc_auto_idx(reg_N, offset, add);

        // base reg is always written on a post-idx?
        REG(reg_N) = index;
    }

#undef calc_auto_idx
#undef calc_add_idx
#undef calc_sub_idx
}