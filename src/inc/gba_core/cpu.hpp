#pragma once
#include <iostream>
#include <queue>
#include <vcruntime_string.h>

#include "gba_core/io.hpp"

#include "util/util.hpp"

#define DIV_MAX 1024

class CPU {
public:
    CPU(IO_Bus *io, bool bootrom_enabled = false);
    ~CPU();

    u8 decode(u32 op);
    u8 decode_thumb(u16 op);
private:
    IO_Bus *io;

    void tick();

    enum op_type_t {
        op_type_REG,
        op_type_SHFT_REG,
        op_type_IMM,
    };

    enum op_code_t {
        op_code_DATA_PROCESSING,
        op_code_LOAD_STORE,
        op_code_BRANCH,
        op_code_COPROCESSOR
    };

    enum op_data_proc_cmd_t {
        op_data_proc_cmd_AND, // binary and
        op_data_proc_cmd_EOR, // binary exclusive or
        op_data_proc_cmd_SUB, // arithemetic subtract
        op_data_proc_cmd_RSB, // arithmetic reverse substract
        op_data_proc_cmd_ADD, // arithemetic add
        op_data_proc_cmd_ADC, // arithemetic add with carry
        op_data_proc_cmd_SBC, // arithemetic subtract with carry
        op_data_proc_cmd_RSC, // arithemetic reverse subtract with carry
        op_data_proc_cmd_TST, // test and
        op_data_proc_cmd_TEQ, // test exclusive or
        op_data_proc_cmd_CMP, // test compare(subtract)
        op_data_proc_cmd_CMN, // test compare(add)
        op_data_proc_cmd_ORR, // binary or
        op_data_proc_cmd_MOV, // move
        op_data_proc_cmd_BIC, // binary bit clear
        op_data_proc_cmd_MVN, // move not
    };

    enum op_data_proc_shift_t {
        op_data_proc_shift_LSL,
        op_data_proc_shift_LSR,
        op_data_proc_shift_ASR,
        op_data_proc_shift_ROR
    };

    enum op_cycle_t {
        op_cycle_CALC_B_DEST, // alu = PC + imm
        op_cycle_FETCH_ALU_AND_LINK,
        op_cycle_WRITE_LR,
    };

    u32 op1;
    u32 op2;
    void prefetch();
    void prefetch(u32 dest);
    void compute_flags(bool set_flags, u32 value);
    void execute();

    u32 calc_shift_operand(op_data_proc_shift_t shift_type, u8 shift_amount, u8 reg_M, bool *carry_out = nullptr);
    void execute_data_processing_command(CPU::op_data_proc_cmd_t command, bool set_flags, u8 reg_N, u8 reg_D, u32 shift_operand, bool shift_carry_out);

    //op implementations
    void data_proc_reg(
        bool condition,
        op_data_proc_cmd_t command,
        bool set_flags,
        u8 reg_N,
        u8 reg_D,
        u8 shift_amount,
        op_data_proc_shift_t shift_type,
        u8 reg_M
    );

    void data_proc_shifted_reg(
        bool condition,
        op_data_proc_cmd_t command,
        bool set_flags,
        u8 reg_N,
        u8 reg_D,
        u8 shift_amount,
        op_data_proc_shift_t shift_type,
        u8 reg_M
    );

    void data_proc_immediate(
        bool condition,
        op_data_proc_cmd_t command,
        bool set_flags,
        u8 reg_N,
        u8 reg_D,
        u8 rotate,
        u8 immediate
    );

    void load_store_shifted_reg(
        bool condition,
        bool pre_index,
        bool add,
        bool word,
        bool write,
        bool is_load,
        u8 reg_N,
        u8 reg_D,
        u8 shift_amount,
        op_data_proc_shift_t shift_type,
        u8 reg_M
    );

    void load_store_immediate(
        bool condition,
        bool pre_index,
        bool add,
        bool word,
        bool write,
        bool is_load,
        u8 reg_N,
        u8 reg_D,
        u16 immediate
    );

    void branch(bool condition, bool link, s32 immediate);
};