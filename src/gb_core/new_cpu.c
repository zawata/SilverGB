#if 0
#include "util/ints.hpp"

#define C_FLAG (Bit::test(AF.b_AF.i_F, 3))
#define H_FLAG (Bit::test(AF.b_AF.i_F, 2))
#define N_FLAG (Bit::test(AF.b_AF.i_F, 1))
#define Z_FLAG (Bit::test(AF.b_AF.i_F, 0))

#define A_REG  (AF.b_AF.A)
#define F_REG  (AF.b_AF.i_F)
#define B_REG  (BC.b_BC.B)
#define C_REG  (BC.b_BC.C)
#define D_REG  (DE.b_DE.D)
#define E_REG  (DE.b_DE.E)
#define H_REG  (HL.b_HL.H)
#define L_REG  (HL.b_HL.L)
#define AF_REG (AF.i_AF)
#define BC_REG (BC.i_BC)
#define DE_REG (DE.i_DE)
#define HL_REG (HL.i_HL)
#define SP_REG (SP)
#define PC_REG (PC)

#define BEGIN_CYCLES \
    switch(op_clock) {\

#define CYCLE(X) \
    case X: {\

#define END_CYCLE \
    }\
    break;\

#define END_CYCLES \
    default: fprintf(stderr, "CYCLE ERROR: %d", __LINE__);\
    }

typedef int op_stat_t;
typedef struct {
    u8 cond;
    union {
        u8  *r8_1;
        u16 *r16_1;
        u8   l8_1;
        u16 *l16_1;
    } op1;
    union {
        u8  *r8_2;
        u16 *r16_2;
        u8   l8_2;
        u16  l16_2;
    } op2;
} op_param_t;

union {
    struct {
        u8 F;
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

//Op Functions
//============
// Nop
//============
op_stat_t no_op();

//============
// Loads
//============
op_stat_t load_r_n(op_param_t);
op_stat_t load_r_r(op_param_t);
op_stat_t load_r_ll(op_param_t);
op_stat_t load_rr_nn(op_param_t);
op_stat_t loadi_rr_r(op_param_t);
op_stat_t loadi_r_rr(op_param_t);
op_stat_t loadd_rr_r(op_param_t);
op_stat_t loadd_r_rr(op_param_t);
op_stat_t load_rr_rr(op_param_t);
op_stat_t load_llnn_r(op_param_t);
op_stat_t load_llnn_rr(op_param_t);
op_stat_t load_ll_n(op_param_t);
op_stat_t load_ll_r(op_param_t);
op_stat_t load_lln_r(op_param_t);
op_stat_t load_llr_r(op_param_t);
op_stat_t load_r_lln(op_param_t);
op_stat_t load_r_llr(op_param_t);
op_stat_t load_rr_rrn(op_param_t);
op_stat_t load_r_llnn(op_param_t);

//====================
// Increment/Decrement
//====================
op_stat_t inc_r(op_param_t);
op_stat_t dec_r(op_param_t);
op_stat_t inc_rr(op_param_t);
op_stat_t dec_rr(op_param_t);
op_stat_t inc_ll(op_param_t);
op_stat_t dec_ll(op_param_t);

//====================
// Add
//====================
op_stat_t add_r_n(op_param_t);
op_stat_t add_r_r(op_param_t);
op_stat_t add_r_ll(op_param_t);

op_stat_t add_rr_n(op_param_t);
op_stat_t add_rr_rr(op_param_t);

op_stat_t adc_r_n(op_param_t);
op_stat_t adc_r_r(op_param_t);
op_stat_t adc_r_ll(op_param_t);

//====================
// Subtract
//====================
op_stat_t sub_r_n(op_param_t);
op_stat_t sub_r(op_param_t);
op_stat_t sub_ll(op_param_t);

op_stat_t sbc_r_n(op_param_t);
op_stat_t sbc_r_r(op_param_t);
op_stat_t sbc_r_ll(op_param_t);

//====================
// And
//====================
op_stat_t and_r_n(op_param_t);
op_stat_t and_r_r(op_param_t);
op_stat_t and_r_ll(op_param_t);

//====================
// Or
//====================
op_stat_t or_r_n(op_param_t);
op_stat_t or_r_r(op_param_t);
op_stat_t or_r_ll(op_param_t);

//====================
// Exclusive Or
//====================
op_stat_t xor_r_n(op_param_t);
op_stat_t xor_r_r(op_param_t);
op_stat_t xor_r_ll(op_param_t);

//====================
// Compare
//====================
op_stat_t cp_r_n(op_param_t);
op_stat_t cp_r_r(op_param_t);
op_stat_t cp_r_ll(op_param_t);

//====================
// Push/Pop
//====================
op_stat_t pop_rr(op_param_t);
op_stat_t pop_af(op_param_t); //special case
op_stat_t push_rr(op_param_t);

//====================
// Rotate
//====================
op_stat_t rla_r(op_param_t);
op_stat_t rra_r(op_param_t);
op_stat_t rlca_r(op_param_t);
op_stat_t rrca_r(op_param_t);
op_stat_t rlc_r(op_param_t);
op_stat_t rlc_ll(op_param_t);
op_stat_t rrc_r(op_param_t);
op_stat_t rrc_ll(op_param_t);
op_stat_t rl_r(op_param_t);
op_stat_t rl_ll(op_param_t);
op_stat_t rr_r(op_param_t);
op_stat_t rr_ll(op_param_t);

//====================
// Shift
//====================
op_stat_t sla_r(op_param_t);
op_stat_t sla_ll(op_param_t);
op_stat_t sra_r(op_param_t);
op_stat_t sra_ll(op_param_t);
op_stat_t srl_r(op_param_t);
op_stat_t srl_ll(op_param_t);

//====================
// Swap
//====================
op_stat_t swap_r(op_param_t);
op_stat_t swap_ll(op_param_t);

//====================
// Test Bit
//====================
op_stat_t bit_b_r(op_param_t);
op_stat_t bit_b_ll(op_param_t);

//====================
// Reset Bit
//====================
op_stat_t res_b_r(op_param_t);
op_stat_t res_b_ll(op_param_t);

//====================
// Set Bit
//====================
op_stat_t set_b_r(op_param_t);
op_stat_t set_b_ll(op_param_t);

//====================
// Decimal Adjust
//====================
op_stat_t daa_r(op_param_t);

//====================
// Complement
//====================
op_stat_t cpl_r(op_param_t);

//====================
// Set/Clear Carry
//====================
op_stat_t scf();
op_stat_t ccf();

//====================
// Halt/Stop
//====================
op_stat_t halt();
op_stat_t stop();

//====================
// Enable/Disable Interrupts
//====================
op_stat_t ei();
op_stat_t di();

//====================
// Reset
//====================
op_stat_t rst_l(op_stat_t loc);

//====================
// Jump
//====================
op_stat_t jump_nn();
op_stat_t jump_ll(op_param_t);
op_stat_t jump_cond_ll(op_param_t);
op_stat_t jump_rel_n();
op_stat_t jump_rel_cond_n(op_param_t);

//====================
// Call
//====================
op_stat_t call_nn();
op_stat_t call_cond_nn(op_param_t);

//====================
// Return
//====================
op_stat_t ret();
op_stat_t reti();
op_stat_t ret_cond(op_param_t);

//====================
// Invalid Op
//====================
op_stat_t invalid_op(u8 op);

enum {
    OP_FINISH,
    OP_CONTINUE,
};

#define pass ((void)0)


#define DECODE pass //decoding happens before the instruction function is executed
#define DELAY  pass

enum op_cond_t {
    NO_COND = 0,
    C_COND,
    NC_COND,
    Z_COND,
    NZ_COND
};

typedef op_stat_t (*op_func)(op_param_t);

typedef struct  {
    op_stat_t func;
    op_param_t params;
} decoded_op_t;

#define loc_1(X) .l16_1 = &X
#define loc_2(X) .l16_2 = &X
#define cond(X) .cond = op_cond_t::X##_COND

decoded_op_t decode[0xF][0xF] = {
    {
//      |function pointer  |condition|op1              |op2                     |timing|instruction
        { no_op           ,{                                            0}},    //    4 NOP
        { load_rr_nn      ,{          .r16_1 = &BC_REG                   }},    //   12 LD    BC, yyxx
        { load_ll_r       ,{          loc_1(BC_REG),    .r8_2 = &A_REG   }},    //    8 LD   (BC), A
        { inc_rr          ,{          .r16_1 = &BC_REG                   }},    //    8 INC   BC
        { inc_r           ,{          .r8_1 = &B_REG                     }},    //    4 INC   B
        { dec_r           ,{          .r8_1 = &B_REG                     }},    //    4 DEC   B
        { load_r_n        ,{          .r8_1 = &B_REG                     }},    //    8 LD    B, xx
        { rlca_r          ,{          .r8_1 = &A_REG                     }},    //    4 RLCA
        { load_llnn_rr    ,{          .r16_1 = &SP_REG                   }},    //   ?? LD   (yyxx), SP
        { add_rr_rr       ,{          .r16_1 = &HL_REG, .r16_2 = &BC_REG }},    //    8 ADD   HL, BC
        { load_r_ll       ,{          .r8_1 = &A_REG,   loc_2(BC_REG)    }},    //    8 LD    A,  (BC)
        { dec_rr          ,{          .r16_1 = &BC_REG                   }},    //    8 DEC   BC
        { inc_r           ,{          .r8_1 = &C_REG                     }},    //    4 INC   C
        { dec_r           ,{          .r8_1 = &C_REG                     }},    //    4 DEC   C
        { load_r_n        ,{          .r8_1 = &C_REG                     }},    //    8 LD    C, xx
        { rrca_r          ,{          .r8_1 = &A_REG                     }},    //    4 RRCA
    },{
        { stop            ,{                                            0}},    //    4 STOP
        { load_rr_nn      ,{          .r16_1 = &DE_REG                   }},    //   12 LD    DE, yyxx
        { load_ll_r       ,{          loc_1(DE_REG),    .r8_2 = &A_REG   }},    //    8 LD   (DE), A
        { inc_rr          ,{          .r16_1 = &DE_REG                   }},    //    8 INC   DE
        { inc_r           ,{          .r8_1  = &D_REG                    }},    //    4 INC   D
        { dec_r           ,{          .r8_1  = &D_REG                    }},    //    4 DEC   D
        { load_r_n        ,{          .r8_1  = &D_REG                    }},    //    8 LD    D, xx
        { rla_r           ,{          .r8_1  = &A_REG                    }},    //    4 RLA
        { jump_rel_n      ,{          0                                  }},    //   12 JR    xx
        { add_rr_rr       ,{          .r16_1 = &HL_REG, .r16_2 = &DE_REG }},    //    8 ADD   HL, DE
        { load_r_ll       ,{          .r8_1  = &A_REG,  loc_2(DE_REG)    }},    //    8 LD A,  (DE)
        { dec_rr          ,{          .r16_1 = &DE_REG                   }},    //    8 DEC DE
        { inc_r           ,{          .r8_1  = &E_REG                    }},    //    4 INC E
        { dec_r           ,{          .r8_1  = &E_REG                    }},    //    4 DEC E
        { load_r_n        ,{          .r8_1  = &E_REG                    }},    //    8 LD E, xx
        { rra_r           ,{          .r8_1  = &A_REG                    }},    //    4 RRA
    },{
        { jump_rel_cond_n ,{ cond(NZ)                                    }},    //   ?? JR NZ, xx
        { load_rr_nn      ,{          .r16_1 = &HL_REG                   }},    //   12 LD HL, yyxx
        { loadi_rr_r      ,{          .r16_1 = &HL_REG, .r8_2 = &A_REG   }},    //    8 LDI (HL), A
        { inc_rr          ,{          .r16_1 = &HL_REG                   }},    //    8 INC HL
        { inc_r           ,{          .r8_1  = &H_REG                    }},    //    4 INC H
        { dec_r           ,{          .r8_1  = &H_REG                    }},    //    4 DEC H
        { load_r_n        ,{          .r8_1  = &H_REG                    }},    //    8 LD H, xx
        { daa_r           ,{          .r8_1  = &A_REG                    }},    //    4 DAA
        { jump_rel_cond_n ,{ cond(Z)                                     }},    //   ?? JR Z, xx
        { add_rr_rr       ,{          .r16_1 = &HL_REG, .r16_2 = &HL_REG }},    //    8 ADD HL, HL
        { loadi_r_rr      ,{          .r8_1  = &A_REG,  .r16_2 = &HL_REG }},    //    8 LDI A,  (HL)
        { dec_rr          ,{          .r16_1 = &HL_REG                   }},    //    8 DEC HL
        { inc_r           ,{          .r8_1  = &L_REG                    }},    //    4 INC L
        { dec_r           ,{          .r8_1  = &L_REG                    }},    //    4 DEC L
        { load_r_n        ,{          .r8_1  = &L_REG                    }},    //    8 LD L, xx
        { cpl_r           ,{          .r8_1  = &A_REG                    }},    //    4 CPL
    },{
        { jump_rel_cond_n ,{ cond(NC)                                    }},    //   ?? JR NC, xx
        { load_rr_nn      ,{          .r16_1 = &SP_REG                   }},    //   12 LD SP, yyxx
        { loadd_rr_r      ,{          .r16_1 = &HL_REG, .r8_2 = &A_REG   }},    //    8 LDD (HL), A
        { inc_rr          ,{          .r16_1 = &SP_REG                   }},    //    8 INC SP
        { inc_ll          ,{          loc_1(HL_REG)                      }},    //   12 INC (HL)
        { dec_ll          ,{          loc_1(HL_REG)                      }},    //   12 DEC (HL)
        { load_ll_n       ,{          loc_1(HL_REG)                      }},    //   12 LD (HL), xx
        { scf             ,{                                            0}},    //    4 SCF
        { jump_rel_cond_n ,{ cond(C)                                     }},    //   ?? JR C, xx
        { add_rr_rr       ,{          .r16_1 = &HL_REG, .r16_1 = &SP_REG }},    //    8 ADD HL, SP
        { loadd_r_rr      ,{          .r8_1  = &A_REG,  loc_2(HL_REG)    }},    //    8 LDD A,  (HL)
        { dec_rr          ,{          .r16_1 = &SP_REG                   }},    //    8 DEC SP
        { inc_r           ,{          .r8_1  = &A_REG                    }},    //    4 INC A
        { dec_r           ,{          .r8_1  = &A_REG                    }},    //    4 DEC A
        { load_r_n        ,{          .r8_1  = &A_REG                    }},    //    8 LD A, xx
        { ccf             ,{                                            0}},    //    4 CCF
    },{
        { load_r_r        ,{          .r8_1  = &B_REG,  .r8_2 = &B_REG   }},    //    4 LD B, B
        { load_r_r        ,{          .r8_1  = &B_REG,  .r8_2 = &C_REG   }},    //    4 LD B, C
        { load_r_r        ,{          .r8_1  = &B_REG,  .r8_2 = &D_REG   }},    //    4 LD B, D
        { load_r_r        ,{          .r8_1  = &B_REG,  .r8_2 = &E_REG   }},    //    4 LD B, E
        { load_r_r        ,{          .r8_1  = &B_REG,  .r8_2 = &H_REG   }},    //    4 LD B, H
        { load_r_r        ,{          .r8_1  = &B_REG,  .r8_2 = &L_REG   }},    //    4 LD B, L
        { load_r_ll       ,{          .r8_1  = &B_REG,  loc_2(HL_REG)    }},    //    8 LD B,  (HL)
        { load_r_r        ,{          .r8_1  = &B_REG,  .r8_2 = &A_REG   }},    //    4 LD B, A
        { load_r_r        ,{          .r8_1  = &C_REG,  .r8_2 = &B_REG   }},    //    4 LD C, B
        { load_r_r        ,{          .r8_1  = &C_REG,  .r8_2 = &C_REG   }},    //    4 LD C, C
        { load_r_r        ,{          .r8_1  = &C_REG,  .r8_2 = &D_REG   }},    //    4 LD C, D
        { load_r_r        ,{          .r8_1  = &C_REG,  .r8_2 = &E_REG   }},    //    4 LD C, E
        { load_r_r        ,{          .r8_1  = &C_REG,  .r8_2 = &H_REG   }},    //    4 LD C, H
        { load_r_r        ,{          .r8_1  = &C_REG,  .r8_2 = &L_REG   }},    //    4 LD C, L
        { load_r_ll       ,{          .r8_1  = &C_REG,  loc_2(HL_REG)    }},    //    8 LD C,  (HL)
        { load_r_r        ,{          .r8_1  = &C_REG,  .r8_2 = &A_REG   }},    //    4 LD C, A
    },{
        { load_r_r        ,{          .r8_1  = &D_REG,  .r8_2 = &B_REG   }},    //    4 LD D, B
        { load_r_r        ,{          .r8_1  = &D_REG,  .r8_2 = &C_REG   }},    //    4 LD D, C
        { load_r_r        ,{          .r8_1  = &D_REG,  .r8_2 = &D_REG   }},    //    4 LD D, D
        { load_r_r        ,{          .r8_1  = &D_REG,  .r8_2 = &E_REG   }},    //    4 LD D, E
        { load_r_r        ,{          .r8_1  = &D_REG,  .r8_2 = &H_REG   }},    //    4 LD D, H
        { load_r_r        ,{          .r8_1  = &D_REG,  .r8_2 = &L_REG   }},    //    4 LD D, L
        { load_r_ll       ,{          .r8_1  = &D_REG,  loc_2(HL_REG)    }},    //    8 LD D,  (HL)
        { load_r_r        ,{          .r8_1  = &D_REG,  .r8_2 = &A_REG   }},    //    4 LD D, A
        { load_r_r        ,{          .r8_1  = &E_REG,  .r8_2 = &B_REG   }},    //    4 LD E, B
        { load_r_r        ,{          .r8_1  = &E_REG,  .r8_2 = &C_REG   }},    //    4 LD E, C
        { load_r_r        ,{          .r8_1  = &E_REG,  .r8_2 = &D_REG   }},    //    4 LD E, D
        { load_r_r        ,{          .r8_1  = &E_REG,  .r8_2 = &E_REG   }},    //    4 LD E, E
        { load_r_r        ,{          .r8_1  = &E_REG,  .r8_2 = &H_REG   }},    //    4 LD E, H
        { load_r_r        ,{          .r8_1  = &E_REG,  .r8_2 = &L_REG   }},    //    4 LD E, L
        { load_r_ll       ,{          .r8_1  = &E_REG,  loc_2(HL_REG)    }},    //    8 LD E,  (HL)
        { load_r_r        ,{          .r8_1  = &E_REG,  .r8_2 = &A_REG   }},    //    4 LD E, A
    },{
        { load_r_r        ,{          .r8_1  = &H_REG,  .r8_2 = &B_REG   }},    //    4 LD H, B
        { load_r_r        ,{          .r8_1  = &H_REG,  .r8_2 = &C_REG   }},    //    4 LD H, C
        { load_r_r        ,{          .r8_1  = &H_REG,  .r8_2 = &D_REG   }},    //    4 LD H, D
        { load_r_r        ,{          .r8_1  = &H_REG,  .r8_2 = &E_REG   }},    //    4 LD H, E
        { load_r_r        ,{          .r8_1  = &H_REG,  .r8_2 = &H_REG   }},    //    4 LD H, H
        { load_r_r        ,{          .r8_1  = &H_REG,  .r8_2 = &L_REG   }},    //    4 LD H, L
        { load_r_ll       ,{          .r8_1  = &H_REG,  loc_2(HL_REG)    }},    //    8 LD H,  (HL)
        { load_r_r        ,{          .r8_1  = &H_REG,  .r8_2 = &A_REG   }},    //    4 LD H, A
        { load_r_r        ,{          .r8_1  = &L_REG,  .r8_2 = &B_REG   }},    //    4 LD L, B
        { load_r_r        ,{          .r8_1  = &L_REG,  .r8_2 = &C_REG   }},    //    4 LD L, C
        { load_r_r        ,{          .r8_1  = &L_REG,  .r8_2 = &D_REG   }},    //    4 LD L, D
        { load_r_r        ,{          .r8_1  = &L_REG,  .r8_2 = &E_REG   }},    //    4 LD L, E
        { load_r_r        ,{          .r8_1  = &L_REG,  .r8_2 = &H_REG   }},    //    4 LD L, H
        { load_r_r        ,{          .r8_1  = &L_REG,  .r8_2 = &L_REG   }},    //    4 LD L, L
        { load_r_ll       ,{          .r8_1  = &L_REG,  loc_2(HL_REG)    }},    //    8 LD L,  (HL)
        { load_r_r        ,{          .r8_1  = &L_REG,  .r8_2 = &A_REG   }},    //    4 LD L, A
    },{
        { load_ll_r       ,{          loc(HL_REG),      .r8_2 = &B_REG   }},    //    8 LD (HL), B
        { load_ll_r       ,{          loc(HL_REG),      .r8_2 = &C_REG   }},    //    8 LD (HL), C
        { load_ll_r       ,{          loc(HL_REG),      .r8_2 = &D_REG   }},    //    8 LD (HL), D
        { load_ll_r       ,{          loc(HL_REG),      .r8_2 = &E_REG   }},    //    8 LD (HL), E
        { load_ll_r       ,{          loc(HL_REG),      .r8_2 = &H_REG   }},    //    8 LD (HL), H
        { load_ll_r       ,{          loc(HL_REG),      .r8_2 = &L_REG   }},    //    8 LD (HL), L
        { halt            ,{                                            0}},    //    4 HALT
        { load_ll_r       ,{          loc(HL_REG),      .r8_2 = &A_REG   }},    //    8 LD (HL), A
        { load_r_r        ,{          .r8_1 = &A_REG,   .r8_2 = &B_REG   }},    //    4 LD A, B
        { load_r_r        ,{          .r8_1 = &A_REG,   .r8_2 = &C_REG   }},    //    4 LD A, C
        { load_r_r        ,{          .r8_1 = &A_REG,   .r8_2 = &D_REG   }},    //    4 LD A, D
        { load_r_r        ,{          .r8_1 = &A_REG,   .r8_2 = &E_REG   }},    //    4 LD A, E
        { load_r_r        ,{          .r8_1 = &A_REG,   .r8_2 = &H_REG   }},    //    4 LD A, H
        { load_r_r        ,{          .r8_1 = &A_REG,   .r8_2 = &L_REG   }},    //    4 LD A, L
        { load_r_ll       ,{          .r8_1 = &A_REG,   loc_2(HL_REG)    }},    //    8 LD A,  (HL)
        { load_r_r        ,{          .r8_1 = &A_REG,   .r8_2 = &A_REG   }},    //    4 LD A, A
    },{
        { add_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &B_REG   }},    //    4 ADD A, B
        { add_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &C_REG   }},    //    4 ADD A, C
        { add_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &D_REG   }},    //    4 ADD A, D
        { add_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &E_REG   }},    //    4 ADD A, E
        { add_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &H_REG   }},    //    4 ADD A, H
        { add_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &L_REG   }},    //    4 ADD A, L
        { add_r_ll        ,{          .r8_1 = &A_REG,   loc_2(HL_REG)    }},    //    8 ADD A,  (HL)
        { add_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &A_REG   }},    //    4 ADD A, A
        { adc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &B_REG   }},    //    4 ADC A, B
        { adc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &C_REG   }},    //    4 ADC A, C
        { adc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &D_REG   }},    //    4 ADC A, D
        { adc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &E_REG   }},    //    4 ADC A, E
        { adc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &H_REG   }},    //    4 ADC A, H
        { adc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &L_REG   }},    //    4 ADC A, L
        { adc_r_ll        ,{          .r8_1 = &A_REG,   loc_2(HL_REG)    }},    //    8 ADC A,  (HL)
        { adc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &A_REG   }},    //    4 ADC A, A
    },{
        { sub_r           ,{          .r8_1 = &A_REG,   .r8_2 = &B_REG   }},    //    4 SUB B
        { sub_r           ,{          .r8_1 = &A_REG,   .r8_2 = &C_REG   }},    //    4 SUB C
        { sub_r           ,{          .r8_1 = &A_REG,   .r8_2 = &D_REG   }},    //    4 SUB D
        { sub_r           ,{          .r8_1 = &A_REG,   .r8_2 = &E_REG   }},    //    4 SUB E
        { sub_r           ,{          .r8_1 = &A_REG,   .r8_2 = &H_REG   }},    //    4 SUB H
        { sub_r           ,{          .r8_1 = &A_REG,   .r8_2 = &L_REG   }},    //    4 SUB L
        { sub_ll          ,{          .r8_1 = &A_REG,   loc_2(HL_REG)    }},    //    8 SUB (HL)
        { sub_r           ,{          .r8_1 = &A_REG,   .r8_2 = &A_REG   }},    //    4 SUB A
        { sbc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &B_REG   }},    //    4 SBC A, B
        { sbc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &C_REG   }},    //    4 SBC A, C
        { sbc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &D_REG   }},    //    4 SBC A, D
        { sbc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &E_REG   }},    //    4 SBC A, E
        { sbc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &H_REG   }},    //    4 SBC A, H
        { sbc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &L_REG   }},    //    4 SBC A, L
        { sbc_r_ll        ,{          .r8_1 = &A_REG,   loc_2(HL_REG)    }},    //    8 SBC A,  (HL)
        { sbc_r_r         ,{          .r8_1 = &A_REG,   .r8_2 = &A_REG   }},    //    4 SBC A, A
    },{
        { and_r           ,{          .r8_1 = &B_REG                     }},    //    4 AND B
        { and_r           ,{          .r8_1 = &C_REG                     }},    //    4 AND C
        { and_r           ,{          .r8_1 = &D_REG                     }},    //    4 AND D
        { and_r           ,{          .r8_1 = &E_REG                     }},    //    4 AND E
        { and_r           ,{          .r8_1 = &H_REG                     }},    //    4 AND H
        { and_r           ,{          .r8_1 = &L_REG                     }},    //    4 AND L
        { and_ll          ,{          loc_1(HL_REG)                      }},    //    8 AND (HL)
        { and_r           ,{          .r8_1 = &A_REG                     }},    //    4 AND A
        { xor_r           ,{          .r8_1 = &B_REG                     }},    //    4 XOR B
        { xor_r           ,{          .r8_1 = &C_REG                     }},    //    4 XOR C
        { xor_r           ,{          .r8_1 = &D_REG                     }},    //    4 XOR D
        { xor_r           ,{          .r8_1 = &E_REG                     }},    //    4 XOR E
        { xor_r           ,{          .r8_1 = &H_REG                     }},    //    4 XOR H
        { xor_r           ,{          .r8_1 = &L_REG                     }},    //    4 XOR L
        { xor_ll          ,{          loc_1(HL_REG)                      }},    //    8 XOR (HL)
        { xor_r           ,{          .r8_1 = &A_REG                     }},    //    4 XOR A
    },{
        { or_r            ,{          .r8_1 = &B_REG                     }},    //    4 OR B
        { or_r            ,{          .r8_1 = &C_REG                     }},    //    4 OR C
        { or_r            ,{          .r8_1 = &D_REG                     }},    //    4 OR D
        { or_r            ,{          .r8_1 = &E_REG                     }},    //    4 OR E
        { or_r            ,{          .r8_1 = &H_REG                     }},    //    4 OR H
        { or_r            ,{          .r8_1 = &L_REG                     }},    //    4 OR L
        { or_ll           ,{          loc_1(HL_REG)                      }},    //    8 OR (HL)
        { or_r            ,{          .r8_1 = &A_REG                     }},    //    4 OR A
        { cp_r            ,{          .r8_1 = &B_REG                     }},    //    4 CP B
        { cp_r            ,{          .r8_1 = &C_REG                     }},    //    4 CP C
        { cp_r            ,{          .r8_1 = &D_REG                     }},    //    4 CP D
        { cp_r            ,{          .r8_1 = &E_REG                     }},    //    4 CP E
        { cp_r            ,{          .r8_1 = &H_REG                     }},    //    4 CP H
        { cp_r            ,{          .r8_1 = &L_REG                     }},    //    4 CP L
        { cp_ll           ,{          loc_1(HL_REG)                      }},    //    8 CP (HL)
        { cp_r            ,{          .r8_1 = &A_REG                     }},    //    4 CP A
    },{
        { ret_cond        ,{ cond(NZ)                                    }},    //   ?? RET NZ
        { pop_rr          ,{          .r16_1 = &BC_REG                   }},    //   12 POP BC
        { jump_cond_ll    ,{ cond(NZ)                                    }},    //   ?? JP NZ yyxx
        { jump_nn         ,{                                            0}},    //   16 JP yyxx
        { call_cond_nn    ,{ cond(NZ)                                    }},    //   ?? CALL NZ yyxx
        { push_rr         ,{          .r16_1 = &BC_REG                   }},    //   16 PUSH BC
        { add_r_n         ,{          .r8_1 = &A_REG                     }},    //    8 ADD A, xx
        { rst_l           ,{          .l8_1 = 0x00                       }},    //   16 RST 00h
        { ret_cond        ,{ cond(Z)                                     }},    //   ?? RET Z
        { ret             ,{                                            0}},    //   16 RET
        { jump_cond_ll    ,{ cond(Z)                                     }},    //   ?? JP Z yyxx
        { 0/*TODO*/       ,{                                            0}},
        { call_cond_nn    ,{ cond(Z)                                     }},    //   ?? CALL Z yyxx
        { call_nn         ,{                                            0}},    //   24 CALL yyxx
        { adc_r_n         ,{          .r8_1 = &A_REG                     }},    //    8 ADCA,  xx
        { rst_l           ,{          .l8_1 = 0x08                       }},    //   16 RST 08
    },{
        { ret_cond        ,{ cond(NC)                                    }},    //   ?? RET NC
        { pop_rr          ,{          .r16_1 = &DE_REG                   }},    //   12 POP DE
        { jump_cond_ll    ,{ cond(NC)                                    }},    //   ?? JP NC yyxx
        { invalid_op      ,{          .l8_1 = 0xd3                       }},    // ---- -----------
        { call_cond_nn    ,{ cond(NC)                                    }},    //   ?? CALL NC yyxx
        { push_rr         ,{          .r16_1 = &DE_REG                   }},    //   12 PUSH DE
        { sub_r_n         ,{          .r8_1 = &A_REG                     }},    //    8 SUB xx
        { rst_l           ,{          .l8_1 = 0x10                       }},    //   16 RST 10h
        { ret_cond        ,{ cond(C)                                     }},    //   ?? RET C
        { reti            ,{                                            0}},    //   16 RETI
        { jump_cond_ll    ,{ cond(C)                                     }},    //   ?? JP C yyxx
        { invalid_op      ,{          .l8_1 = 0xdb                       }},    // ---- -----------
        { call_cond_nn    ,{ cond(C)                                     }},    //   ?? CALL C yyxx
        { invalid_op      ,{          .l8_1 = 0xdd                       }},    // ---- -----------
        { sbc_r_n         ,{          .r8_1 = &A_REG                     }},    //    8 SBC A, xx
        { rst_l           ,{          .l8_1 = 0x18                       }},    //   16 RST 18h
    },{
        { load_lln_r      ,{          .r8_1 = &A_REG                     }},    //   12 LD ($FF00 + xx), A
        { pop_rr          ,{          .r16_1 = &HL_REG                   }},    //   12 POP HL
        { load_llr_r      ,{          .r8_1 = &C_REG,   .r8_2 = &A_REG   }},    //    8 LD ($FF00 + C), A
        { invalid_op      ,{          .l8_1 = 0xe3                       }},    // ---- -----------
        { invalid_op      ,{          .l8_1 = 0xe4                       }},    // ---- -----------
        { push_rr         ,{          .r16_1 = &HL_REG                   }},    //   16 PUSH HL
        { and_r_n         ,{          .r8_1 = &A_REG                     }},    //    8 AND xx
        { rst_l           ,{          .l8_1 = 0x20                       }},    //   16 RST 20h
        { add_rr_n        ,{          .r16_1 = &SP_REG                   }},    //    8 ADD SP, xx
        { jump_ll         ,{          loc_1(HL_REG)                      }},    //    4 JP (HL)
        { load_llnn_r     ,{          .r8_1 = &A_REG                     }},    //   16 LD (yyxx), A
        { invalid_op      ,{          .l8_1 = 0xeb                       }},    // ---- -----------
        { invalid_op      ,{          .l8_1 = 0xec                       }},    // ---- -----------
        { invalid_op      ,{          .l8_1 = 0xed                       }},    // ---- -----------
        { xor_r_n         ,{          .r8_1 = &A_REG                     }},    //    8 XOR xx
        { rst_l           ,{          .l8_1 = 0x28                       }},    //   16 RST 28h
    },{
        { load_r_lln      ,{          .r8_1 = &A_REG                     }},    //   12 LD A,             ($FF00+xx)
        { pop_af          ,{          .r16_1 = &AF_REG                   }},    //   12 POP AF
        { load_r_llr      ,{          .r8_1 = &A_REG,   .r8_2 = &C_REG   }},    //    8 LD A,             (FF00+C)
        { di              ,{                                            0}},    //    4 DI
        { invalid_op      ,{          .l8_1 = 0xf4                       }},    // ---- -----------
        { push_rr         ,{          .r16_1 = &AF_REG                   }},    //   16 PUSH AF
        { or_r_n          ,{          .r8_1 = &A_REG                     }},    //    8 OR xx
        { rst_l           ,{          .l8_1 = 0x30                       }},    //   16 RST 30h
        { load_rr_rrn     ,{          .r16_1 = &HL_REG, .r16_2 = &SP_REG }},    //   12 LD HL, SP+xx
        { load_rr_rr      ,{          .r16_1 = &SP_REG, .r16_2 = &HL_REG }},    //    8 LD SP, HL
        { load_r_llnn     ,{          .r8_1 = &A_REG                     }},    //   16 LD A, (yyxx)
        { ei              ,{                                            0}},    //    4 EI
        { invalid_op      ,{          .l8_1 = 0xfc                       }},    // ---- -----------
        { invalid_op      ,{          .l8_1 = 0xfd                       }},    // ---- -----------
        { cp_r_n          ,{          .r8_1 = &A_REG                     }},    //    8 CP xx
        { rst_l           ,{          .l8_1 = 0x38                       }},    //   16 RST 38 h
    }
};

decoded_op_t decode_cb[0xF][0xF] = {
    {
        { rlc_r           ,{          .r8_1 = &B_REG                     }}, //    4 RLC B
        { rlc_r           ,{          .r8_1 = &C_REG                     }}, //    4 RLC C
        { rlc_r           ,{          .r8_1 = &D_REG                     }}, //    4 RLC D
        { rlc_r           ,{          .r8_1 = &E_REG                     }}, //    4 RLC E
        { rlc_r           ,{          .r8_1 = &H_REG                     }}, //    4 RLC H
        { rlc_r           ,{          .r8_1 = &L_REG                     }}, //    4 RLC L
        { rlc_ll          ,{          loc_1(HL_REG)                      }}, //    4 RLC (HL)
        { rlc_r           ,{          .r8_1 = &A_REG                     }}, //    4 RLC A
        { rrc_r           ,{          .r8_1 = &B_REG                     }}, //    4 RRC B
        { rrc_r           ,{          .r8_1 = &C_REG                     }}, //    4 RRC C
        { rrc_r           ,{          .r8_1 = &D_REG                     }}, //    4 RRC D
        { rrc_r           ,{          .r8_1 = &E_REG                     }}, //    4 RRC E
        { rrc_r           ,{          .r8_1 = &H_REG                     }}, //    4 RRC H
        { rrc_r           ,{          .r8_1 = &L_REG                     }}, //    4 RRC L
        { rrc_ll          ,{          loc_1(HL_REG)                      }}, //    4 RRC (HL)
        { rrc_r           ,{          .r8_1 = &A_REG                     }}, //    4 RRC A
    },{
        { rl_r            ,{          .r8_1 = &B_REG                     }}, //    4 RL B
        { rl_r            ,{          .r8_1 = &C_REG                     }}, //    4 RL C
        { rl_r            ,{          .r8_1 = &D_REG                     }}, //    4 RL D
        { rl_r            ,{          .r8_1 = &E_REG                     }}, //    4 RL E
        { rl_r            ,{          .r8_1 = &H_REG                     }}, //    4 RL H
        { rl_r            ,{          .r8_1 = &L_REG                     }}, //    4 RL L
        { rl_ll           ,{          loc_1(HL_REG)                      }}, //    4 RL (HL)
        { rl_r            ,{          .r8_1 = &A_REG                     }}, //    4 RL A
        { rr_r            ,{          .r8_1 = &B_REG                     }}, //    4 RR B
        { rr_r            ,{          .r8_1 = &C_REG                     }}, //    4 RR C
        { rr_r            ,{          .r8_1 = &D_REG                     }}, //    4 RR D
        { rr_r            ,{          .r8_1 = &E_REG                     }}, //    4 RR E
        { rr_r            ,{          .r8_1 = &H_REG                     }}, //    4 RR H
        { rr_r            ,{          .r8_1 = &L_REG                     }}, //    4 RR L
        { rr_ll           ,{          loc_1(HL_REG)                      }}, //    4 RR (HL)
        { rr_r            ,{          .r8_1 = &A_REG                     }}, //    4 RR A
    },{
        { sla_r           ,{          .r8_1 = &B_REG                     }}, //    4 SLA B
        { sla_r           ,{          .r8_1 = &C_REG                     }}, //    4 SLA C
        { sla_r           ,{          .r8_1 = &D_REG                     }}, //    4 SLA D
        { sla_r           ,{          .r8_1 = &E_REG                     }}, //    4 SLA E
        { sla_r           ,{          .r8_1 = &H_REG                     }}, //    4 SLA H
        { sla_r           ,{          .r8_1 = &L_REG                     }}, //    4 SLA L
        { sla_ll          ,{          loc_1(HL_REG)                      }}, //    4 SLA (HL)
        { sla_r           ,{          .r8_1 = &A_REG                     }}, //    4 SLA A
        { sra_r           ,{          .r8_1 = &B_REG                     }}, //    4 SRA B
        { sra_r           ,{          .r8_1 = &C_REG                     }}, //    4 SRA C
        { sra_r           ,{          .r8_1 = &D_REG                     }}, //    4 SRA D
        { sra_r           ,{          .r8_1 = &E_REG                     }}, //    4 SRA E
        { sra_r           ,{          .r8_1 = &H_REG                     }}, //    4 SRA H
        { sra_r           ,{          .r8_1 = &L_REG                     }}, //    4 SRA L
        { sra_ll          ,{          loc_1(HL_REG)                      }}, //    4 SRA (HL)
        { sra_r           ,{          .r8_1 = &A_REG                     }}, //    4 SRA A
    },{
        { swap_r          ,{          .r8_1 = &B_REG                     }}, //    4 SWAP B
        { swap_r          ,{          .r8_1 = &C_REG                     }}, //    4 SWAP C
        { swap_r          ,{          .r8_1 = &D_REG                     }}, //    4 SWAP D
        { swap_r          ,{          .r8_1 = &E_REG                     }}, //    4 SWAP E
        { swap_r          ,{          .r8_1 = &H_REG                     }}, //    4 SWAP H
        { swap_r          ,{          .r8_1 = &L_REG                     }}, //    4 SWAP L
        { swap_ll         ,{          loc_1(HL_REG)                      }}, //    4 SWAP (HL)
        { swap_r          ,{          .r8_1 = &A_REG                     }}, //    4 SWAP A
        { srl_r           ,{          .r8_1 = &B_REG                     }}, //    4 SRL B
        { srl_r           ,{          .r8_1 = &C_REG                     }}, //    4 SRL C
        { srl_r           ,{          .r8_1 = &D_REG                     }}, //    4 SRL D
        { srl_r           ,{          .r8_1 = &E_REG                     }}, //    4 SRL E
        { srl_r           ,{          .r8_1 = &H_REG                     }}, //    4 SRL H
        { srl_r           ,{          .r8_1 = &L_REG                     }}, //    4 SRL L
        { srl_ll          ,{          loc_1(HL_REG)                      }}, //    4 SRL (HL)
        { srl_r           ,{          .r8_1 = &A_REG                     }}, //    4 SRL A
    },{
        { bit_b_r         ,{          .l8_1 = 0,        .r8_2 = &B_REG   }}, //    4 BIT 0, B
        { bit_b_r         ,{          .l8_1 = 0,        .r8_2 = &C_REG   }}, //    4 BIT 0, C
        { bit_b_r         ,{          .l8_1 = 0,        .r8_2 = &D_REG   }}, //    4 BIT 0, D
        { bit_b_r         ,{          .l8_1 = 0,        .r8_2 = &E_REG   }}, //    4 BIT 0, E
        { bit_b_r         ,{          .l8_1 = 0,        .r8_2 = &H_REG   }}, //    4 BIT 0, H
        { bit_b_r         ,{          .l8_1 = 0,        .r8_2 = &L_REG   }}, //    4 BIT 0, L
        { bit_b_ll        ,{          .l8_1 = 0,        loc_2(HL_REG)    }}, //    4 BIT 0, (HL)
        { bit_b_r         ,{          .l8_1 = 0,        .r8_2 = &A_REG   }}, //    4 BIT 0, A
        { bit_b_r         ,{          .l8_1 = 1,        .r8_2 = &B_REG   }}, //    4 BIT 1, B
        { bit_b_r         ,{          .l8_1 = 1,        .r8_2 = &C_REG   }}, //    4 BIT 1, C
        { bit_b_r         ,{          .l8_1 = 1,        .r8_2 = &D_REG   }}, //    4 BIT 1, D
        { bit_b_r         ,{          .l8_1 = 1,        .r8_2 = &E_REG   }}, //    4 BIT 1, E
        { bit_b_r         ,{          .l8_1 = 1,        .r8_2 = &H_REG   }}, //    4 BIT 1, H
        { bit_b_r         ,{          .l8_1 = 1,        .r8_2 = &L_REG   }}, //    4 BIT 1, L
        { bit_b_ll        ,{          .l8_1 = 1,        loc_2(HL_REG)    }}, //    4 BIT 1, (HL)
        { bit_b_r         ,{          .l8_1 = 1,        .r8_2 = &A_REG   }}, //    4 BIT 1, A
    },{
        { bit_b_r         ,{          .l8_1 = 2,        .r8_2 = &B_REG   }}, //    4 BIT 2, B
        { bit_b_r         ,{          .l8_1 = 2,        .r8_2 = &C_REG   }}, //    4 BIT 2, C
        { bit_b_r         ,{          .l8_1 = 2,        .r8_2 = &D_REG   }}, //    4 BIT 2, D
        { bit_b_r         ,{          .l8_1 = 2,        .r8_2 = &E_REG   }}, //    4 BIT 2, E
        { bit_b_r         ,{          .l8_1 = 2,        .r8_2 = &H_REG   }}, //    4 BIT 2, H
        { bit_b_r         ,{          .l8_1 = 2,        .r8_2 = &L_REG   }}, //    4 BIT 2, L
        { bit_b_ll        ,{          .l8_1 = 2,        loc_2(HL_REG)    }}, //    4 BIT 2, (HL)
        { bit_b_r         ,{          .l8_1 = 2,        .r8_2 = &A_REG   }}, //    4 BIT 2, A
        { bit_b_r         ,{          .l8_1 = 3,        .r8_2 = &B_REG   }}, //    4 BIT 3, B
        { bit_b_r         ,{          .l8_1 = 3,        .r8_2 = &C_REG   }}, //    4 BIT 3, C
        { bit_b_r         ,{          .l8_1 = 3,        .r8_2 = &D_REG   }}, //    4 BIT 3, D
        { bit_b_r         ,{          .l8_1 = 3,        .r8_2 = &E_REG   }}, //    4 BIT 3, E
        { bit_b_r         ,{          .l8_1 = 3,        .r8_2 = &H_REG   }}, //    4 BIT 3, H
        { bit_b_r         ,{          .l8_1 = 3,        .r8_2 = &L_REG   }}, //    4 BIT 3, L
        { bit_b_ll        ,{          .l8_1 = 3,        loc_2(HL_REG)    }}, //    4 BIT 3, (HL)
        { bit_b_r         ,{          .l8_1 = 3,        .r8_2 = &A_REG   }}, //    4 BIT 3, A
    },{
        { bit_b_r         ,{          .l8_1 = 4,        .r8_2 = &B_REG   }}, //    4 BIT 4, B
        { bit_b_r         ,{          .l8_1 = 4,        .r8_2 = &C_REG   }}, //    4 BIT 4, C
        { bit_b_r         ,{          .l8_1 = 4,        .r8_2 = &D_REG   }}, //    4 BIT 4, D
        { bit_b_r         ,{          .l8_1 = 4,        .r8_2 = &E_REG   }}, //    4 BIT 4, E
        { bit_b_r         ,{          .l8_1 = 4,        .r8_2 = &H_REG   }}, //    4 BIT 4, H
        { bit_b_r         ,{          .l8_1 = 4,        .r8_2 = &L_REG   }}, //    4 BIT 4, L
        { bit_b_ll        ,{          .l8_1 = 4,        loc_2(HL_REG)    }}, //    4 BIT 4, (HL)
        { bit_b_r         ,{          .l8_1 = 4,        .r8_2 = &A_REG   }}, //    4 BIT 4, A
        { bit_b_r         ,{          .l8_1 = 5,        .r8_2 = &B_REG   }}, //    4 BIT 5, B
        { bit_b_r         ,{          .l8_1 = 5,        .r8_2 = &C_REG   }}, //    4 BIT 5, C
        { bit_b_r         ,{          .l8_1 = 5,        .r8_2 = &D_REG   }}, //    4 BIT 5, D
        { bit_b_r         ,{          .l8_1 = 5,        .r8_2 = &E_REG   }}, //    4 BIT 5, E
        { bit_b_r         ,{          .l8_1 = 5,        .r8_2 = &H_REG   }}, //    4 BIT 5, H
        { bit_b_r         ,{          .l8_1 = 5,        .r8_2 = &L_REG   }}, //    4 BIT 5, L
        { bit_b_ll        ,{          .l8_1 = 5,        loc_2(HL_REG)    }}, //    4 BIT 5, (HL)
        { bit_b_r         ,{          .l8_1 = 5,        .r8_2 = &A_REG   }}, //    4 BIT 5, A
    },{
        { bit_b_r         ,{          .l8_1 = 6,        .r8_2 = &B_REG   }}, //    4 BIT 6, B
        { bit_b_r         ,{          .l8_1 = 6,        .r8_2 = &C_REG   }}, //    4 BIT 6, C
        { bit_b_r         ,{          .l8_1 = 6,        .r8_2 = &D_REG   }}, //    4 BIT 6, D
        { bit_b_r         ,{          .l8_1 = 6,        .r8_2 = &E_REG   }}, //    4 BIT 6, E
        { bit_b_r         ,{          .l8_1 = 6,        .r8_2 = &H_REG   }}, //    4 BIT 6, H
        { bit_b_r         ,{          .l8_1 = 6,        .r8_2 = &L_REG   }}, //    4 BIT 6, L
        { bit_b_ll        ,{          .l8_1 = 6,        loc_2(HL_REG)    }}, //    4 BIT 6, (HL)
        { bit_b_r         ,{          .l8_1 = 6,        .r8_2 = &A_REG   }}, //    4 BIT 6, A
        { bit_b_r         ,{          .l8_1 = 7,        .r8_2 = &B_REG   }}, //    4 BIT 7, B
        { bit_b_r         ,{          .l8_1 = 7,        .r8_2 = &C_REG   }}, //    4 BIT 7, C
        { bit_b_r         ,{          .l8_1 = 7,        .r8_2 = &D_REG   }}, //    4 BIT 7, D
        { bit_b_r         ,{          .l8_1 = 7,        .r8_2 = &E_REG   }}, //    4 BIT 7, E
        { bit_b_r         ,{          .l8_1 = 7,        .r8_2 = &H_REG   }}, //    4 BIT 7, H
        { bit_b_r         ,{          .l8_1 = 7,        .r8_2 = &L_REG   }}, //    4 BIT 7, L
        { bit_b_ll        ,{          .l8_1 = 7,        loc_2(HL_REG)    }}, //    4 BIT 7, (HL)
        { bit_b_r         ,{          .l8_1 = 7,        .r8_2 = &A_REG   }}, //    4 BIT 7, A
    },{
        { res_b_r         ,{          .l8_1 = 0,        .r8_2 = &B_REG   }}, //    4 RES 0, B
        { res_b_r         ,{          .l8_1 = 0,        .r8_2 = &C_REG   }}, //    4 RES 0, C
        { res_b_r         ,{          .l8_1 = 0,        .r8_2 = &D_REG   }}, //    4 RES 0, D
        { res_b_r         ,{          .l8_1 = 0,        .r8_2 = &E_REG   }}, //    4 RES 0, E
        { res_b_r         ,{          .l8_1 = 0,        .r8_2 = &H_REG   }}, //    4 RES 0, H
        { res_b_r         ,{          .l8_1 = 0,        .r8_2 = &L_REG   }}, //    4 RES 0, L
        { res_b_ll        ,{          .l8_1 = 0,        loc_2(HL_REG)    }}, //    4 RES 0, (HL)
        { res_b_r         ,{          .l8_1 = 0,        .r8_2 = &A_REG   }}, //    4 RES 0, A
        { res_b_r         ,{          .l8_1 = 1,        .r8_2 = &B_REG   }}, //    4 RES 1, B
        { res_b_r         ,{          .l8_1 = 1,        .r8_2 = &C_REG   }}, //    4 RES 1, C
        { res_b_r         ,{          .l8_1 = 1,        .r8_2 = &D_REG   }}, //    4 RES 1, D
        { res_b_r         ,{          .l8_1 = 1,        .r8_2 = &E_REG   }}, //    4 RES 1, E
        { res_b_r         ,{          .l8_1 = 1,        .r8_2 = &H_REG   }}, //    4 RES 1, H
        { res_b_r         ,{          .l8_1 = 1,        .r8_2 = &L_REG   }}, //    4 RES 1, L
        { res_b_ll        ,{          .l8_1 = 1,        loc_2(HL_REG)    }}, //    4 RES 1, (HL)
        { res_b_r         ,{          .l8_1 = 1,        .r8_2 = &A_REG   }}, //    4 RES 1, A
    },{
        { res_b_r         ,{          .l8_1 = 2,        .r8_2 = &B_REG   }}, //    4 RES 2, B
        { res_b_r         ,{          .l8_1 = 2,        .r8_2 = &C_REG   }}, //    4 RES 2, C
        { res_b_r         ,{          .l8_1 = 2,        .r8_2 = &D_REG   }}, //    4 RES 2, D
        { res_b_r         ,{          .l8_1 = 2,        .r8_2 = &E_REG   }}, //    4 RES 2, E
        { res_b_r         ,{          .l8_1 = 2,        .r8_2 = &H_REG   }}, //    4 RES 2, H
        { res_b_r         ,{          .l8_1 = 2,        .r8_2 = &L_REG   }}, //    4 RES 2, L
        { res_b_ll        ,{          .l8_1 = 2,        loc_2(HL_REG)    }}, //    4 RES 2, (HL)
        { res_b_r         ,{          .l8_1 = 2,        .r8_2 = &A_REG   }}, //    4 RES 2, A
        { res_b_r         ,{          .l8_1 = 3,        .r8_2 = &B_REG   }}, //    4 RES 3, B
        { res_b_r         ,{          .l8_1 = 3,        .r8_2 = &C_REG   }}, //    4 RES 3, C
        { res_b_r         ,{          .l8_1 = 3,        .r8_2 = &D_REG   }}, //    4 RES 3, D
        { res_b_r         ,{          .l8_1 = 3,        .r8_2 = &E_REG   }}, //    4 RES 3, E
        { res_b_r         ,{          .l8_1 = 3,        .r8_2 = &H_REG   }}, //    4 RES 3, H
        { res_b_r         ,{          .l8_1 = 3,        .r8_2 = &L_REG   }}, //    4 RES 3, L
        { res_b_ll        ,{          .l8_1 = 3,        loc_2(HL_REG)    }}, //    4 RES 3, (HL)
        { res_b_r         ,{          .l8_1 = 3,        .r8_2 = &A_REG   }}, //    4 RES 3, A
    },{
        { res_b_r         ,{          .l8_1 = 4,        .r8_2 = &B_REG   }}, //    4 RES 4, B
        { res_b_r         ,{          .l8_1 = 4,        .r8_2 = &C_REG   }}, //    4 RES 4, C
        { res_b_r         ,{          .l8_1 = 4,        .r8_2 = &D_REG   }}, //    4 RES 4, D
        { res_b_r         ,{          .l8_1 = 4,        .r8_2 = &E_REG   }}, //    4 RES 4, E
        { res_b_r         ,{          .l8_1 = 4,        .r8_2 = &H_REG   }}, //    4 RES 4, H
        { res_b_r         ,{          .l8_1 = 4,        .r8_2 = &L_REG   }}, //    4 RES 4, L
        { res_b_ll        ,{          .l8_1 = 4,        loc_2(HL_REG)    }}, //    4 RES 4, (HL)
        { res_b_r         ,{          .l8_1 = 4,        .r8_2 = &A_REG   }}, //    4 RES 4, A
        { res_b_r         ,{          .l8_1 = 5,        .r8_2 = &B_REG   }}, //    4 RES 5, B
        { res_b_r         ,{          .l8_1 = 5,        .r8_2 = &C_REG   }}, //    4 RES 5, C
        { res_b_r         ,{          .l8_1 = 5,        .r8_2 = &D_REG   }}, //    4 RES 5, D
        { res_b_r         ,{          .l8_1 = 5,        .r8_2 = &E_REG   }}, //    4 RES 5, E
        { res_b_r         ,{          .l8_1 = 5,        .r8_2 = &H_REG   }}, //    4 RES 5, H
        { res_b_r         ,{          .l8_1 = 5,        .r8_2 = &L_REG   }}, //    4 RES 5, L
        { res_b_ll        ,{          .l8_1 = 5,        loc_2(HL_REG)    }}, //    4 RES 5, (HL)
        { res_b_r         ,{          .l8_1 = 5,        .r8_2 = &A_REG   }}, //    4 RES 5, A
    },{
        { res_b_r         ,{          .l8_1 = 6,        .r8_2 = &B_REG   }}, //    4 RES 6, B
        { res_b_r         ,{          .l8_1 = 6,        .r8_2 = &C_REG   }}, //    4 RES 6, C
        { res_b_r         ,{          .l8_1 = 6,        .r8_2 = &D_REG   }}, //    4 RES 6, D
        { res_b_r         ,{          .l8_1 = 6,        .r8_2 = &E_REG   }}, //    4 RES 6, E
        { res_b_r         ,{          .l8_1 = 6,        .r8_2 = &H_REG   }}, //    4 RES 6, H
        { res_b_r         ,{          .l8_1 = 6,        .r8_2 = &L_REG   }}, //    4 RES 6, L
        { res_b_ll        ,{          .l8_1 = 6,        loc_2(HL_REG)    }}, //    4 RES 6, (HL)
        { res_b_r         ,{          .l8_1 = 6,        .r8_2 = &A_REG   }}, //    4 RES 6, A
        { res_b_r         ,{          .l8_1 = 7,        .r8_2 = &B_REG   }}, //    4 RES 7, B
        { res_b_r         ,{          .l8_1 = 7,        .r8_2 = &C_REG   }}, //    4 RES 7, C
        { res_b_r         ,{          .l8_1 = 7,        .r8_2 = &D_REG   }}, //    4 RES 7, D
        { res_b_r         ,{          .l8_1 = 7,        .r8_2 = &E_REG   }}, //    4 RES 7, E
        { res_b_r         ,{          .l8_1 = 7,        .r8_2 = &H_REG   }}, //    4 RES 7, H
        { res_b_r         ,{          .l8_1 = 7,        .r8_2 = &L_REG   }}, //    4 RES 7, L
        { res_b_ll        ,{          .l8_1 = 7,        loc_2(HL_REG)    }}, //    4 RES 7, (HL)
        { res_b_r         ,{          .l8_1 = 7,        .r8_2 = &A_REG   }}, //    4 RES 7, A
    },{
        { set_b_r         ,{          .l8_1 = 0,        .r8_2 = &B_REG   }}, //    4 SET 0, B
        { set_b_r         ,{          .l8_1 = 0,        .r8_2 = &C_REG   }}, //    4 SET 0, C
        { set_b_r         ,{          .l8_1 = 0,        .r8_2 = &D_REG   }}, //    4 SET 0, D
        { set_b_r         ,{          .l8_1 = 0,        .r8_2 = &E_REG   }}, //    4 SET 0, E
        { set_b_r         ,{          .l8_1 = 0,        .r8_2 = &H_REG   }}, //    4 SET 0, H
        { set_b_r         ,{          .l8_1 = 0,        .r8_2 = &L_REG   }}, //    4 SET 0, L
        { set_b_ll        ,{          .l8_1 = 0,        loc_2(HL_REG)    }}, //    4 SET 0, (HL)
        { set_b_r         ,{          .l8_1 = 0,        .r8_2 = &A_REG   }}, //    4 SET 0, A
        { set_b_r         ,{          .l8_1 = 1,        .r8_2 = &B_REG   }}, //    4 SET 1, B
        { set_b_r         ,{          .l8_1 = 1,        .r8_2 = &C_REG   }}, //    4 SET 1, C
        { set_b_r         ,{          .l8_1 = 1,        .r8_2 = &D_REG   }}, //    4 SET 1, D
        { set_b_r         ,{          .l8_1 = 1,        .r8_2 = &E_REG   }}, //    4 SET 1, E
        { set_b_r         ,{          .l8_1 = 1,        .r8_2 = &H_REG   }}, //    4 SET 1, H
        { set_b_r         ,{          .l8_1 = 1,        .r8_2 = &L_REG   }}, //    4 SET 1, L
        { set_b_ll        ,{          .l8_1 = 1,        loc_2(HL_REG)    }}, //    4 SET 1, (HL)
        { set_b_r         ,{          .l8_1 = 1,        .r8_2 = &A_REG   }}, //    4 SET 1, A
    },{
        { set_b_r         ,{          .l8_1 = 2,        .r8_2 = &B_REG   }}, //    4 SET 2, B
        { set_b_r         ,{          .l8_1 = 2,        .r8_2 = &C_REG   }}, //    4 SET 2, C
        { set_b_r         ,{          .l8_1 = 2,        .r8_2 = &D_REG   }}, //    4 SET 2, D
        { set_b_r         ,{          .l8_1 = 2,        .r8_2 = &E_REG   }}, //    4 SET 2, E
        { set_b_r         ,{          .l8_1 = 2,        .r8_2 = &H_REG   }}, //    4 SET 2, H
        { set_b_r         ,{          .l8_1 = 2,        .r8_2 = &L_REG   }}, //    4 SET 2, L
        { set_b_ll        ,{          .l8_1 = 2,        loc_2(HL_REG)    }}, //    4 SET 2, (HL)
        { set_b_r         ,{          .l8_1 = 2,        .r8_2 = &A_REG   }}, //    4 SET 2, A
        { set_b_r         ,{          .l8_1 = 3,        .r8_2 = &B_REG   }}, //    4 SET 3, B
        { set_b_r         ,{          .l8_1 = 3,        .r8_2 = &C_REG   }}, //    4 SET 3, C
        { set_b_r         ,{          .l8_1 = 3,        .r8_2 = &D_REG   }}, //    4 SET 3, D
        { set_b_r         ,{          .l8_1 = 3,        .r8_2 = &E_REG   }}, //    4 SET 3, E
        { set_b_r         ,{          .l8_1 = 3,        .r8_2 = &H_REG   }}, //    4 SET 3, H
        { set_b_r         ,{          .l8_1 = 3,        .r8_2 = &L_REG   }}, //    4 SET 3, L
        { set_b_ll        ,{          .l8_1 = 3,        loc_2(HL_REG)    }}, //    4 SET 3, (HL)
        { set_b_r         ,{          .l8_1 = 3,        .r8_2 = &A_REG   }}, //    4 SET 3, A
    },{
        { set_b_r         ,{          .l8_1 = 4,        .r8_2 = &B_REG   }}, //    4 SET 4, B
        { set_b_r         ,{          .l8_1 = 4,        .r8_2 = &C_REG   }}, //    4 SET 4, C
        { set_b_r         ,{          .l8_1 = 4,        .r8_2 = &D_REG   }}, //    4 SET 4, D
        { set_b_r         ,{          .l8_1 = 4,        .r8_2 = &E_REG   }}, //    4 SET 4, E
        { set_b_r         ,{          .l8_1 = 4,        .r8_2 = &H_REG   }}, //    4 SET 4, H
        { set_b_r         ,{          .l8_1 = 4,        .r8_2 = &L_REG   }}, //    4 SET 4, L
        { set_b_ll        ,{          .l8_1 = 4,        loc_2(HL_REG)    }}, //    4 SET 4, (HL)
        { set_b_r         ,{          .l8_1 = 4,        .r8_2 = &A_REG   }}, //    4 SET 4, A
        { set_b_r         ,{          .l8_1 = 5,        .r8_2 = &B_REG   }}, //    4 SET 5, B
        { set_b_r         ,{          .l8_1 = 5,        .r8_2 = &C_REG   }}, //    4 SET 5, C
        { set_b_r         ,{          .l8_1 = 5,        .r8_2 = &D_REG   }}, //    4 SET 5, D
        { set_b_r         ,{          .l8_1 = 5,        .r8_2 = &E_REG   }}, //    4 SET 5, E
        { set_b_r         ,{          .l8_1 = 5,        .r8_2 = &H_REG   }}, //    4 SET 5, H
        { set_b_r         ,{          .l8_1 = 5,        .r8_2 = &L_REG   }}, //    4 SET 5, L
        { set_b_ll        ,{          .l8_1 = 5,        loc_2(HL_REG)    }}, //    4 SET 5, (HL)
        { set_b_r         ,{          .l8_1 = 5,        .r8_2 = &A_REG   }}, //    4 SET 5, A
    },{
        { set_b_r         ,{          .l8_1 = 6,        .r8_2 = &B_REG   }}, //    4 SET 6, B
        { set_b_r         ,{          .l8_1 = 6,        .r8_2 = &C_REG   }}, //    4 SET 6, C
        { set_b_r         ,{          .l8_1 = 6,        .r8_2 = &D_REG   }}, //    4 SET 6, D
        { set_b_r         ,{          .l8_1 = 6,        .r8_2 = &E_REG   }}, //    4 SET 6, E
        { set_b_r         ,{          .l8_1 = 6,        .r8_2 = &H_REG   }}, //    4 SET 6, H
        { set_b_r         ,{          .l8_1 = 6,        .r8_2 = &L_REG   }}, //    4 SET 6, L
        { set_b_ll        ,{          .l8_1 = 6,        loc_2(HL_REG)    }}, //    4 SET 6, (HL)
        { set_b_r         ,{          .l8_1 = 6,        .r8_2 = &A_REG   }}, //    4 SET 6, A
        { set_b_r         ,{          .l8_1 = 7,        .r8_2 = &B_REG   }}, //    4 SET 7, B
        { set_b_r         ,{          .l8_1 = 7,        .r8_2 = &C_REG   }}, //    4 SET 7, C
        { set_b_r         ,{          .l8_1 = 7,        .r8_2 = &D_REG   }}, //    4 SET 7, D
        { set_b_r         ,{          .l8_1 = 7,        .r8_2 = &E_REG   }}, //    4 SET 7, E
        { set_b_r         ,{          .l8_1 = 7,        .r8_2 = &H_REG   }}, //    4 SET 7, H
        { set_b_r         ,{          .l8_1 = 7,        .r8_2 = &L_REG   }}, //    4 SET 7, L
        { set_b_ll        ,{          .l8_1 = 7,        loc_2(HL_REG)    }}, //    4 SET 7, (HL)
        { set_b_r         ,{          .l8_1 = 7,        .r8_2 = &A_REG   }}, //    4 SET 7, A
    }
};

op_stat_t push(op_stat_t stat) {
    //0 decode
    //1 delay
    //2 upper byte push
    //3 lower byte push

    BEGIN_CYCLES
    CYCLE(0)
        //decode
    END_CYCLE
    CYCLE(1)
        //delay
    END_CYCLE
    CYCLE(2)
        //upper byte push
    END_CYCLE
    CYCLE(3)
        //lower byte push
    END_CYCLE
    END_CYCLES
}

typedef op_stat_t (*op_func)(op_stat_t);
bool op_exec;
u8 op_clock;
op_func current_op;
op_stat_t current_op_stat;

void cpu_loop() {
    if(!op_exec) {
        u8 io_byte      = fetch(PC_REG);
        current_op      = decode[io_byte];
    }

    current_op(current_op_stat)

}

#endif
