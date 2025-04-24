#pragma once
#include <nowide/iostream.hpp>

#include "gba_core/io.hpp"

#include "arm_instruction.hh"

class CPU {
public:
    CPU(IO_Bus *io, bool bootrom_enabled = false);
    ~CPU();

    void tick();

    u8   decode(u32 op);
    u8   decode_thumb(u16 op);

private:
    IO_Bus *io;

    u32     op1;
    u32     op2;
    void    prefetch16();
    void    prefetch16(u16 dest);
    void    prefetch32();
    void    prefetch32(u32 dest);
    void    compute_flags(bool set_flags, u32 value);
    void    execute();

    u32     calc_shift_operand(Arm::ShiftType shift_type, u8 shift_amount, u8 reg_M, bool *carry_out = nullptr);

    bool    EvaluateCondition(Arm::Condition condition);

    // arm op interpreters
    void    branch(Arm::Branch const &i);
    void    branch_and_exchange(Arm::BranchAndExchange const &i);
    void    data_proc_reg(Arm::DataProcessing const &i);
    void    psr_transfer(Arm::PSRTransfer const &i);
    void    data_proc_shifted_reg(Arm::DataProcessing const &i);
    void    data_proc_immediate(Arm::DataProcessing const &i);
    void    single_data_transfer_shifted_reg(Arm::SingleDataTransfer const &i);
    void    single_data_transfer_immediate(Arm::SingleDataTransfer const &i);
    void    halfword_data_transfer_shifted_reg(Arm::HalfwordDataTransfer const &i);
    void    halfword_data_transfer_immediate(Arm::HalfwordDataTransfer const &i);
    void    block_data_transfer(Arm::BlockDataTransfer const &i);
    void    swap(Arm::Swap const &i);

    void    exec_dp_op(Arm::DataProcessing const &i, u32 shift_operand, bool shift_carry_out);
    void    exec_sdt_load_op(Arm::SingleDataTransfer const &i, u32 offset);
    void    exec_sdt_store_op(Arm::SingleDataTransfer const &i, u32 offset);
    void    exec_hwsd_load_op(Arm::HalfwordDataTransfer const &i, u32 offset);
    void    exec_hwsd_store_op(Arm::HalfwordDataTransfer const &i, u32 offset);
    void    exec_bdt_load_op(Arm::BlockDataTransfer const &i);
    void    exec_bdt_store_op(Arm::BlockDataTransfer const &i);
};
