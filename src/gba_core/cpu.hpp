#pragma once
#include <queue>

#include <nowide/iostream.hpp>

#include "gba_core/io.hpp"

#include "arm_instruction.hh"
#include "util/util.hpp"

class CPU {
public:
    CPU(IO_Bus *io, bool bootrom_enabled = false);
    ~CPU();

    void tick();

    u8 decode(u32 op);
    u8 decode_thumb(u16 op);
private:
    IO_Bus *io;

    u32 op1;
    u32 op2;
    void prefetch16();
    void prefetch16(u16 dest);
    void prefetch32();
    void prefetch32(u32 dest);
    void compute_flags(bool set_flags, u32 value);
    void execute();

    u32 calc_shift_operand(Arm::ShiftType shift_type, u8 shift_amount, u8 reg_M, bool *carry_out = nullptr);

    bool EvaluateCondition(Arm::Condition condition);

    //arm op interpreters
    void branch(Arm::Instruction instr);
    void branch_and_exchange(Arm::Instruction instr);
    void psr_transfer(Arm::Instruction instr);
    void data_proc_reg(Arm::Instruction instr);
    void data_proc_shifted_reg(Arm::Instruction instr);
    void data_proc_immediate(Arm::Instruction instr);
    void load_store_shifted_reg(Arm::Instruction instr);
    void load_store_immediate(Arm::Instruction instr);

    void exec_dp_op(Arm::Instruction instr, u32 shift_operand, bool shift_carry_out);
    void exec_ls_op(
      bool load,
      bool pre_index,
      bool add,
      bool word,
      bool write,
      u8 reg_N,
      u8 reg_D,
      u32 offset
    );
};