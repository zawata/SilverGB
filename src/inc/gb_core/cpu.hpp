#pragma once

#include "gb_core/io.hpp"
#include "util/util.hpp"

#include <iostream>

#define DIV_MAX 1024

class CPU {
public:
    //for public use
    struct registers_t {
        u16 AF;
        u16 BC;
        u16 DE;
        u16 HL;
        u16 SP;
        u16 PC;
    };

    CPU(IO_Bus *io, bool bootrom_enabled);
    ~CPU();

    bool tick();

    u8 decode(u8 op);
    std::string getOpString(u16 PC);
    std::string getCBOpString(u16 PC);

    registers_t getRegisters();

private:
    void on_div16();
    void on_div64();
    void on_div256();
    void on_div1024();

    //used for disassembly
    std::string i8(u16 PC);
    std::string i16(u16 PC);
    std::string format(std::string fmt, u16 PC);

    IO_Bus *io;

    u8 inst_clocks;
    u16 cpu_counter;
    //globally track div values because writes to the TIMA register
    u16 old_div = 0, new_div = 0;

    //registers:
    union {
        struct {
            union {
                struct {
                    u8 __u : 4; //unused
                    u8 c   : 1; //carry
                    u8 h   : 1; //half_carry
                    u8 n   : 1; //subtraction
                    u8 z   : 1; //zero_flag
                } b_F;
                u8 i_F;
            };
            u8 A;
        } b_AF;
        u16 i_AF;
    } AF;

    union {
        struct {
            u8 C;
            u8 B;
        } b_BC;
        u16 i_BC;
    } BC;

    union {
        struct {
            u8 E;
            u8 D;
        } b_DE;
        u16 i_DE;
    } DE;

    union {
        struct {
            u8 L;
            u8 H;
        } b_HL;
        u16 i_HL;
    } HL;

    u16 SP;
    u16 PC;

    //flags
    bool
        IME = 0,
        is_halted = false,
        halt_bug  = false,
        is_stopped = false,
        ei_ime_enable = false;

    //Execution Functions
    u8 *decode_reg(u8 reg);

    u8  fetch_8();
    u16 fetch_16();

    void stack_push(u16 n);
    u16  stack_pop();

    //Op Functions
    //============
    // Nop
    //============
    u8 no_op();

    //============
    // Loads
    //============
    u8 load_r_n(u8 *r1);
    u8 load_r_r(u8 *r1, u8 *r2);
    u8 load_r_ll(u8 *r1, u16 loc);
    u8 load_rr_nn(u16 *r1);
    u8 loadi_rr_r(u16 *r1, u8 *r2);
    u8 loadi_r_rr(u8 *r1, u16 *r2);
    u8 loadd_rr_r(u16 *r1, u8 *r2);
    u8 loadd_r_rr(u8 *r1, u16 *r2);
    u8 load_rr_rr(u16 *r1, u16 *r2);
    u8 load_llnn_r(u8 *r1);
    u8 load_llnn_rr(u16 *r1);
    u8 load_ll_n(u16 loc);
    u8 load_ll_r(u16 loc, u8 *r1);
    u8 load_lln_r(u8 *r1);
    u8 load_llr_r(u8 *r1, u8 *r2);
    u8 load_r_lln(u8 *r1);
    u8 load_r_llr(u8 *r1, u8 *r2);
    u8 load_rr_rrn(u16 *r1, u16 *r2);
    u8 load_r_llnn(u8 *r1);

    //====================
    // Increment/Decrement
    //====================
    u8 inc_r(u8 *r1);
    u8 dec_r(u8 *r1);
    u8 inc_rr(u16 *r1);
    u8 dec_rr(u16 *r1);
    u8 inc_ll(u16 loc);
    u8 dec_ll(u16 loc);

    //====================
    // Add
    //====================
    u8 add_r_n(u8 *r1);
    u8 add_r_r(u8 *r1, u8 *r2);
    u8 add_r_ll(u8 *r1, u16 loc);

    u8 add_rr_n(u16 *r1);
    u8 add_rr_rr(u16 *r1, u16 *r2);

    u8 adc_r_n(u8 *r1);
    u8 adc_r_r(u8 *r1, u8 *r2);
    u8 adc_r_ll(u8 *r1, u16 loc);

    //====================
    // Subtract
    //====================
    u8 sub_r_n(u8 *r1);
    u8 sub_r(u8 *r1, u8 *r2);
    u8 sub_ll(u8 *r1, u16 loc);

    u8 sbc_r_n(u8 *r1);
    u8 sbc_r_r(u8 *r1, u8 *r2);
    u8 sbc_r_ll(u8 *r1, u16 loc);

    //====================
    // And
    //====================
    u8 and_r_n(u8 *r1);
    u8 and_r_r(u8 *r1, u8 *r2);
    u8 and_r_ll(u8 *r1, u16 loc);

    //====================
    // Or
    //====================
    u8 or_r_n(u8 *r1);
    u8 or_r_r(u8 *r1, u8 *r2);
    u8 or_r_ll(u8 *r1, u16 loc);

    //====================
    // Exclusive Or
    //====================
    u8 xor_r_n(u8 *r1);
    u8 xor_r_r(u8 *r1, u8 *r2);
    u8 xor_r_ll(u8 *r1, u16 loc);

    //====================
    // Compare
    //====================
    u8 cp_r_n(u8 *r1);
    u8 cp_r_r(u8 *r1, u8 *r2);
    u8 cp_r_ll(u8 *r1, u16 loc);

    //====================
    // Push/Pop
    //====================
    u8 pop_rr(u16 *r1);
    u8 pop_af(u16 *r1); //special case
    u8 push_rr(u16 *r1);

    //====================
    // Rotate
    //====================
    u8 rla_r(u8 *r1);
    u8 rra_r(u8 *r1);
    u8 rlca_r(u8 *r1);
    u8 rrca_r(u8 *r1);
    u8 rlc_r(u8 *r1);
    u8 rlc_ll(u16 loc);
    u8 rrc_r(u8 *r1);
    u8 rrc_ll(u16 loc);
    u8 rl_r(u8 *r1);
    u8 rl_ll(u16 loc);
    u8 rr_r(u8 *r1);
    u8 rr_ll(u16 loc);

    //====================
    // Shift
    //====================
    u8 sla_r(u8 *reg);
    u8 sla_ll(u16 loc);
    u8 sra_r(u8 *reg);
    u8 sra_ll(u16 loc);
    u8 srl_r(u8 *reg);
    u8 srl_ll(u16 loc);

    //====================
    // Swap
    //====================
    u8 swap_r(u8 *reg);
    u8 swap_ll(u16 loc);

    //====================
    // Test Bit
    //====================
    u8 bit_b_r(u8 bit, u8 *reg);
    u8 bit_b_ll(u8 bit, u16 loc);

    //====================
    // Reset Bit
    //====================
    u8 res_b_r(u8 bit, u8 *reg);
    u8 res_b_ll(u8 bit, u16 loc);

    //====================
    // Set Bit
    //====================
    u8 set_b_r(u8 bit, u8 *reg);
    u8 set_b_ll(u8 bit, u16 loc);

    //====================
    // Decimal Adjust
    //====================
    u8 daa_r(u8 *r1);

    //====================
    // Complement
    //====================
    u8 cpl_r(u8 *r1);

    //====================
    // Set/Clear Carry
    //====================
    u8 scf();
    u8 ccf();

    //====================
    // Halt/Stop
    //====================
    u8 halt();
    u8 stop();

    //====================
    // Enable/Disable Interrupts
    //====================
    u8 ei();
    u8 di();

    //====================
    // Reset
    //====================
    u8 rst_l(u8 loc);

    //====================
    // Jump
    //====================
    u8 jump_nn();
    u8 jump_ll(u16 loc);
    u8 jump_cond_ll(bool cond);
    u8 jump_rel_n();
    u8 jump_rel_cond_n(bool cond);

    //====================
    // Call
    //====================
    u8 call_nn();
    u8 call_cond_nn(bool cond);

    //====================
    // Return
    //====================
    u8 ret();
    u8 reti();
    u8 ret_cond(bool cond);

    //====================
    // Invalid Op
    //====================
    u8 invalid_op(u8 op);
};