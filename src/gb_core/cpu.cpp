#include "gb_core/cpu.hpp"
#include "gb_core/defs.hpp"

#include "util/bit.hpp"

#define C_FLAG (this->AF.b_AF.b_F.c)
#define H_FLAG (this->AF.b_AF.b_F.h)
#define N_FLAG (this->AF.b_AF.b_F.n)
#define Z_FLAG (this->AF.b_AF.b_F.z)

#define C_COND (this->AF.b_AF.b_F.c == 1)
#define NC_COND (!C_COND)

#define Z_COND (this->AF.b_AF.b_F.z == 1)
#define NZ_COND (!Z_COND)

#define A_REG  this->AF.b_AF.A
#define F_REG  this->AF.b_AF.i_F
#define B_REG  this->BC.b_BC.B
#define C_REG  this->BC.b_BC.C
#define D_REG  this->DE.b_DE.D
#define E_REG  this->DE.b_DE.E
#define H_REG  this->HL.b_HL.H
#define L_REG  this->HL.b_HL.L
#define AF_REG this->AF.i_AF
#define BC_REG this->BC.i_BC
#define DE_REG this->DE.i_DE
#define HL_REG this->HL.i_HL
#define SP_REG this->SP
#define PC_REG this->PC

inline bool check_half_carry_8 ( u8 x,  u8 y,        u16 r) { return (x^y^r)   & 0x10; }
inline bool check_half_carry_8 ( u8 x,  u8 y, u8  z, u16 r) { return (x^y^z^r) & 0x10; }
inline bool      check_carry_8 ( u8 x,  u8 y,        u16 r) { return (x^y^r)   & 0x100; }
inline bool      check_carry_8 ( u8 x,  u8 y, u8  z, u16 r) { return (x^y^z^r) & 0x100; }
inline bool check_half_carry_16(u16 x, u16 y,        u32 r) { return (x^y^r)   & 0x1000; }
inline bool check_half_carry_16(u16 x, u16 y, u16 z, u32 r) { return (x^y^z^r) & 0x1000; }
inline bool      check_carry_16(u16 x, u16 y,        u32 r) { return (x^y^r)   & 0x10000; }
inline bool      check_carry_16(u16 x, u16 y, u16 z, u32 r) { return (x^y^z^r) & 0x10000; }

CPU::CPU(IO_Bus *io, bool bootrom_enabled = false) :
io(io),
inst_clocks(0),
cpu_counter(0),
IME(true) {
    if(bootrom_enabled) {
        AF_REG = 0x0000;
        BC_REG = 0x0000;
        DE_REG = 0x0000;
        HL_REG = 0x0000;
        SP_REG = 0x0000;
        PC_REG = 0x0000;
    } else {
        std::cout << "Starting CPU without bootrom not supported!" << std::endl;
        AF_REG = 0x01b0;
        BC_REG = 0x0013;
        DE_REG = 0x00D8;
        HL_REG = 0x014D;
        SP_REG = 0xFFFE;
        PC_REG = 0x0100;
    }
}

CPU::~CPU() {}

CPU::registers_t CPU::getRegisters() {
    //Thanks microsoft...
#if defined(_MSC_VER)
    CPU::registers_t r = {
        AF_REG,
        BC_REG,
        DE_REG,
        HL_REG,
        SP_REG,
        PC_REG
    };

    return r;
#else
    return (CPU::registers_t) {
        AF_REG,
        BC_REG,
        DE_REG,
        HL_REG,
        SP_REG,
        PC_REG
    };
#endif
}

//TODO: stop
bool CPU::tick() {
    new_div = io->cpu_inc_DIV();
    if(Bit::fallen(old_div, new_div, 3)) on_div16();
    if(Bit::fallen(old_div, new_div, 5)) on_div64();
    if(Bit::fallen(old_div, new_div, 7)) on_div256();
    if(Bit::fallen(old_div, new_div, 9)) on_div1024();
    old_div = new_div;

    if(!inst_clocks) { //used to properly time the instruction execution
        u8 int_pc = 0, int_val = 0;
        bool int_set = false;
        if(io->cpu_check_interrupts() & IO_Bus::Interrupt::VBLANK_INT) {
            int_pc = VBLANK_INT_OFFSET;
            int_val = IO_Bus::Interrupt::VBLANK_INT;
        } else if(io->cpu_check_interrupts() & IO_Bus::Interrupt::LCD_STAT_INT) {
             int_pc = LCD_STAT_INT_OFFSET;
            int_val = IO_Bus::Interrupt::LCD_STAT_INT;
        } else if(io->cpu_check_interrupts() & IO_Bus::Interrupt::TIMER_INT) {
            int_pc = TIMER_INT_OFFSET;
            int_val = IO_Bus::Interrupt::TIMER_INT;
        } else if(io->cpu_check_interrupts() & IO_Bus::Interrupt::SERIAL_INT) {
            int_pc = SERIAL_INT_OFFSET;
            int_val = IO_Bus::Interrupt::SERIAL_INT;
        } else if(io->cpu_check_interrupts() & IO_Bus::Interrupt::JOYPAD_INT) {
            int_pc = JOYPAD_INT_OFFSET;
            int_val = IO_Bus::Interrupt::JOYPAD_INT;
        }

        if(int_val) {
            if(is_halted) is_halted = false;

            //ISR transfer summary:
            // disable Interrupts
            // push the PC
            // set the PC to the interrupt offset
            // unset the interrupt flag
            // set clocks for ISR transfer
            if(IME) {
                IME = 0;
                stack_push(PC_REG);
                io->cpu_unset_interrupt((IO_Bus::Interrupt)int_val);
                PC_REG = int_pc;
                inst_clocks = 20;
                int_set = true;
            }
        }

        if(!int_set && !is_halted) {
            bool old_ei_ime_enable = ei_ime_enable;

            inst_clocks = decode(fetch_8());

            if(old_ei_ime_enable && ei_ime_enable) {
                ei_ime_enable = false;
                IME=1;
            }
        }

        // prevent inst_clocks from underflowing during HALTs.
        // also keep CPU operating on proper M cycles.
        if(is_halted) inst_clocks = 4;
    }
    inst_clocks--;

    //if inst_clocks is 0 here, then the next clock will execute an instruction
    return inst_clocks == 0;
}

inline void CPU::on_div16() {
    if(io->cpu_get_TAC_cs() == 16) io->cpu_inc_TIMA();
}

inline void CPU::on_div64() {
    if(io->cpu_get_TAC_cs() == 64) io->cpu_inc_TIMA();
}

inline void CPU::on_div256() {
    if(io->cpu_get_TAC_cs() == 256) io->cpu_inc_TIMA();
}

inline void CPU::on_div1024() {
    if(io->cpu_get_TAC_cs() == 1024) io->cpu_inc_TIMA();
}

u8 CPU::decode(u8 op) {
     //std::cout << "Instruction 0x" << as_hex(PC_REG-1) << ": " << getOpString(PC_REG-1) << std::endl;

    switch(op) {
        case 0x00: return no_op();                       //   4  NOP
        case 0x01: return load_rr_nn(&BC_REG);           //  12  LD BC, yyxx
        case 0x02: return load_ll_r(BC_REG, &A_REG);     //   8  LD (BC), A
        case 0x03: return inc_rr(&BC_REG);               //   8  INC BC
        case 0x04: return inc_r(&B_REG);                 //   4  INC B
        case 0x05: return dec_r(&B_REG);                 //   4  DEC B
        case 0x06: return load_r_n(&B_REG);              //   8  LD B, xx
        case 0x07: return rlca_r(&A_REG);                //   4  RLCA
        case 0x08: return load_llnn_rr(&SP_REG);         //  ??  LD (yyxx), SP
        case 0x09: return add_rr_rr(&HL_REG, &BC_REG);   //   8  ADD HL, BC
        case 0x0a: return load_r_ll(&A_REG, BC_REG);     //   8  LD A, (BC)
        case 0x0b: return dec_rr(&BC_REG);               //   8  DEC BC
        case 0x0c: return inc_r(&C_REG);                 //   4  INC C
        case 0x0d: return dec_r(&C_REG);                 //   4  DEC C
        case 0x0e: return load_r_n(&C_REG);              //   8  LD C, xx
        case 0x0f: return rrca_r(&A_REG);                //   4  RRCA
        case 0x10: return stop();                        //   4  STOP
        case 0x11: return load_rr_nn(&DE_REG);           //  12  LD DE, yyxx
        case 0x12: return load_ll_r(DE_REG, &A_REG);     //   8  LD (DE), A
        case 0x13: return inc_rr(&DE_REG);               //   8  INC DE
        case 0x14: return inc_r(&D_REG);                 //   4  INC D
        case 0x15: return dec_r(&D_REG);                 //   4  DEC D
        case 0x16: return load_r_n(&D_REG);              //   8  LD D, xx
        case 0x17: return rla_r(&A_REG);                 //   4  RLA
        case 0x18: return jump_rel_n();                  //  12  JR xx
        case 0x19: return add_rr_rr(&HL_REG, &DE_REG);   //   8  ADD HL, DE
        case 0x1a: return load_r_ll(&A_REG, DE_REG);     //   8  LD A,(DE)
        case 0x1b: return dec_rr(&DE_REG);               //   8  DEC DE
        case 0x1c: return inc_r(&E_REG);                 //   4  INC E
        case 0x1d: return dec_r(&E_REG);                 //   4  DEC E
        case 0x1e: return load_r_n(&E_REG);              //   8  LD E, xx
        case 0x1f: return rra_r(&A_REG);                 //   4  RRA
        case 0x20: return jump_rel_cond_n(NZ_COND);      //  ??  JR NZ, xx
        case 0x21: return load_rr_nn(&HL_REG);           //  12  LD HL, yyxx
        case 0x22: return loadi_rr_r(&HL_REG, &A_REG);   //   8  LDI (HL),A
        case 0x23: return inc_rr(&HL_REG);               //   8  INC HL
        case 0x24: return inc_r(&H_REG);                 //   4  INC H
        case 0x25: return dec_r(&H_REG);                 //   4  DEC H
        case 0x26: return load_r_n(&H_REG);              //   8  LD H, xx
        case 0x27: return daa_r(&A_REG);                 //   4  DAA
        case 0x28: return jump_rel_cond_n(Z_COND);       //  ??  JR Z, xx
        case 0x29: return add_rr_rr(&HL_REG, &HL_REG);   //   8  ADD HL, HL
        case 0x2a: return loadi_r_rr(&A_REG, &HL_REG);   //   8  LDI A, (HL)
        case 0x2b: return dec_rr(&HL_REG);               //   8  DEC HL
        case 0x2c: return inc_r(&L_REG);                 //   4  INC L
        case 0x2d: return dec_r(&L_REG);                 //   4  DEC L
        case 0x2e: return load_r_n(&L_REG);              //   8  LD L, xx
        case 0x2f: return cpl_r(&A_REG);                 //   4  CPL
        case 0x30: return jump_rel_cond_n(NC_COND);      //  ??  JR NC, xx
        case 0x31: return load_rr_nn(&SP_REG);           //  12  LD SP, yyxx
        case 0x32: return loadd_rr_r(&HL_REG, &A_REG);   //   8  LDD (HL), A
        case 0x33: return inc_rr(&SP_REG);               //   8  INC SP
        case 0x34: return inc_ll(HL_REG);                //  12  INC (HL)
        case 0x35: return dec_ll(HL_REG);                //  12  DEC (HL)
        case 0x36: return load_ll_n(HL_REG);             //  12  LD (HL), xx
        case 0x37: return scf();                         //   4  SCF
        case 0x38: return jump_rel_cond_n(C_COND);       //  ??  JR C, xx
        case 0x39: return add_rr_rr(&HL_REG, &SP_REG);   //   8  ADD HL, SP
        case 0x3a: return loadd_r_rr(&A_REG, &HL_REG);   //   8  LDD A, (HL)
        case 0x3b: return dec_rr(&SP_REG);               //   8  DEC SP
        case 0x3c: return inc_r(&A_REG);                 //   4  INC A
        case 0x3d: return dec_r(&A_REG);                 //   4  DEC A
        case 0x3e: return load_r_n(&A_REG);              //   8  LD A, xx
        case 0x3f: return ccf();                         //   4  CCF
        case 0x40: return load_r_r(&B_REG, &B_REG);      //   4  LD B, B
        case 0x41: return load_r_r(&B_REG, &C_REG);      //   4  LD B, C
        case 0x42: return load_r_r(&B_REG, &D_REG);      //   4  LD B, D
        case 0x43: return load_r_r(&B_REG, &E_REG);      //   4  LD B, E
        case 0x44: return load_r_r(&B_REG, &H_REG);      //   4  LD B, H
        case 0x45: return load_r_r(&B_REG, &L_REG);      //   4  LD B, L
        case 0x46: return load_r_ll(&B_REG, HL_REG);     //   8  LD B, (HL)
        case 0x47: return load_r_r(&B_REG, &A_REG);      //   4  LD B, A
        case 0x48: return load_r_r(&C_REG, &B_REG);      //   4  LD C, B
        case 0x49: return load_r_r(&C_REG, &C_REG);      //   4  LD C, C
        case 0x4a: return load_r_r(&C_REG, &D_REG);      //   4  LD C, D
        case 0x4b: return load_r_r(&C_REG, &E_REG);      //   4  LD C, E
        case 0x4c: return load_r_r(&C_REG, &H_REG);      //   4  LD C, H
        case 0x4d: return load_r_r(&C_REG, &L_REG);      //   4  LD C, L
        case 0x4e: return load_r_ll(&C_REG, HL_REG);     //   8  LD C, (HL)
        case 0x4f: return load_r_r(&C_REG, &A_REG);      //   4  LD C, A
        case 0x50: return load_r_r(&D_REG, &B_REG);      //   4  LD D, B
        case 0x51: return load_r_r(&D_REG, &C_REG);      //   4  LD D, C
        case 0x52: return load_r_r(&D_REG, &D_REG);      //   4  LD D, D
        case 0x53: return load_r_r(&D_REG, &E_REG);      //   4  LD D, E
        case 0x54: return load_r_r(&D_REG, &H_REG);      //   4  LD D, H
        case 0x55: return load_r_r(&D_REG, &L_REG);      //   4  LD D, L
        case 0x56: return load_r_ll(&D_REG, HL_REG);     //   8  LD D, (HL)
        case 0x57: return load_r_r(&D_REG, &A_REG);      //   4  LD D, A
        case 0x58: return load_r_r(&E_REG, &B_REG);      //   4  LD E, B
        case 0x59: return load_r_r(&E_REG, &C_REG);      //   4  LD E, C
        case 0x5a: return load_r_r(&E_REG, &D_REG);      //   4  LD E, D
        case 0x5b: return load_r_r(&E_REG, &E_REG);      //   4  LD E, E
        case 0x5c: return load_r_r(&E_REG, &H_REG);      //   4  LD E, H
        case 0x5d: return load_r_r(&E_REG, &L_REG);      //   4  LD E, L
        case 0x5e: return load_r_ll(&E_REG, HL_REG);     //   8  LD E, (HL)
        case 0x5f: return load_r_r(&E_REG, &A_REG);      //   4  LD E, A
        case 0x60: return load_r_r(&H_REG, &B_REG);      //   4  LD H, B
        case 0x61: return load_r_r(&H_REG, &C_REG);      //   4  LD H, C
        case 0x62: return load_r_r(&H_REG, &D_REG);      //   4  LD H, D
        case 0x63: return load_r_r(&H_REG, &E_REG);      //   4  LD H, E
        case 0x64: return load_r_r(&H_REG, &H_REG);      //   4  LD H, H
        case 0x65: return load_r_r(&H_REG, &L_REG);      //   4  LD H, L
        case 0x66: return load_r_ll(&H_REG, HL_REG);     //   8  LD H, (HL)
        case 0x67: return load_r_r(&H_REG, &A_REG);      //   4  LD H, A
        case 0x68: return load_r_r(&L_REG, &B_REG);      //   4  LD L, B
        case 0x69: return load_r_r(&L_REG, &C_REG);      //   4  LD L, C
        case 0x6a: return load_r_r(&L_REG, &D_REG);      //   4  LD L, D
        case 0x6b: return load_r_r(&L_REG, &E_REG);      //   4  LD L, E
        case 0x6c: return load_r_r(&L_REG, &H_REG);      //   4  LD L, H
        case 0x6d: return load_r_r(&L_REG, &L_REG);      //   4  LD L, L
        case 0x6e: return load_r_ll(&L_REG, HL_REG);     //   8  LD L, (HL)
        case 0x6F: return load_r_r(&L_REG, &A_REG);      //   4  LD L, A
        case 0x70: return load_ll_r(HL_REG, &B_REG);     //   8  LD (HL), B
        case 0x71: return load_ll_r(HL_REG, &C_REG);     //   8  LD (HL), C
        case 0x72: return load_ll_r(HL_REG, &D_REG);     //   8  LD (HL), D
        case 0x73: return load_ll_r(HL_REG, &E_REG);     //   8  LD (HL), E
        case 0x74: return load_ll_r(HL_REG, &H_REG);     //   8  LD (HL), H
        case 0x75: return load_ll_r(HL_REG, &L_REG);     //   8  LD (HL), L
        case 0x76: return halt();                        //   4  HALT
        case 0x77: return load_ll_r(HL_REG, &A_REG);     //   8  LD (HL), A
        case 0x78: return load_r_r(&A_REG, &B_REG);      //   4  LD A, B
        case 0x79: return load_r_r(&A_REG, &C_REG);      //   4  LD A, C
        case 0x7a: return load_r_r(&A_REG, &D_REG);      //   4  LD A, D
        case 0x7b: return load_r_r(&A_REG, &E_REG);      //   4  LD A, E
        case 0x7c: return load_r_r(&A_REG, &H_REG);      //   4  LD A, H
        case 0x7d: return load_r_r(&A_REG, &L_REG);      //   4  LD A, L
        case 0x7e: return load_r_ll(&A_REG, HL_REG);     //   8  LD A, (HL)
        case 0x7f: return load_r_r(&A_REG, &A_REG);      //   4  LD A, A
        case 0x80: return add_r_r(&A_REG, &B_REG);       //   4  ADD A, B
        case 0x81: return add_r_r(&A_REG, &C_REG);       //   4  ADD A, C
        case 0x82: return add_r_r(&A_REG, &D_REG);       //   4  ADD A, D
        case 0x83: return add_r_r(&A_REG, &E_REG);       //   4  ADD A, E
        case 0x84: return add_r_r(&A_REG, &H_REG);       //   4  ADD A, H
        case 0x85: return add_r_r(&A_REG, &L_REG);       //   4  ADD A, L
        case 0x86: return add_r_ll(&A_REG, HL_REG);      //   8  ADD A, (HL)
        case 0x87: return add_r_r(&A_REG, &A_REG);       //   4  ADD A, A
        case 0x88: return adc_r_r(&A_REG, &B_REG);       //   4  ADC A, B
        case 0x89: return adc_r_r(&A_REG, &C_REG);       //   4  ADC A, C
        case 0x8a: return adc_r_r(&A_REG, &D_REG);       //   4  ADC A, D
        case 0x8b: return adc_r_r(&A_REG, &E_REG);       //   4  ADC A, E
        case 0x8c: return adc_r_r(&A_REG, &H_REG);       //   4  ADC A, H
        case 0x8d: return adc_r_r(&A_REG, &L_REG);       //   4  ADC A, L
        case 0x8e: return adc_r_ll(&A_REG, HL_REG);      //   8  ADC A, (HL)
        case 0x8f: return adc_r_r(&A_REG, &A_REG);       //   4  ADC A, A
        case 0x90: return sub_r(&A_REG, &B_REG);         //   4  SUB B
        case 0x91: return sub_r(&A_REG, &C_REG);         //   4  SUB C
        case 0x92: return sub_r(&A_REG, &D_REG);         //   4  SUB D
        case 0x93: return sub_r(&A_REG, &E_REG);         //   4  SUB E
        case 0x94: return sub_r(&A_REG, &H_REG);         //   4  SUB H
        case 0x95: return sub_r(&A_REG, &L_REG);         //   4  SUB L
        case 0x96: return sub_ll(&A_REG, HL_REG);        //   8  SUB (HL)
        case 0x97: return sub_r(&A_REG, &A_REG);         //   4  SUB A
        case 0x98: return sbc_r_r(&A_REG, &B_REG);       //   4  SBC A, B
        case 0x99: return sbc_r_r(&A_REG, &C_REG);       //   4  SBC A, C
        case 0x9a: return sbc_r_r(&A_REG, &D_REG);       //   4  SBC A, D
        case 0x9b: return sbc_r_r(&A_REG, &E_REG);       //   4  SBC A, E
        case 0x9c: return sbc_r_r(&A_REG, &H_REG);       //   4  SBC A, H
        case 0x9d: return sbc_r_r(&A_REG, &L_REG);       //   4  SBC A, L
        case 0x9e: return sbc_r_ll(&A_REG, HL_REG);      //   8  SBC A, (HL)
        case 0x9f: return sbc_r_r(&A_REG, &A_REG);       //   4  SBC A, A
        case 0xa0: return and_r_r(&A_REG, &B_REG);       //   4  AND B
        case 0xa1: return and_r_r(&A_REG, &C_REG);       //   4  AND C
        case 0xa2: return and_r_r(&A_REG, &D_REG);       //   4  AND D
        case 0xa3: return and_r_r(&A_REG, &E_REG);       //   4  AND E
        case 0xa4: return and_r_r(&A_REG, &H_REG);       //   4  AND H
        case 0xa5: return and_r_r(&A_REG, &L_REG);       //   4  AND L
        case 0xa6: return and_r_ll(&A_REG, HL_REG);      //   8  AND (HL)
        case 0xa7: return and_r_r(&A_REG, &A_REG);       //   4  AND A
        case 0xa8: return xor_r_r(&A_REG, &B_REG);       //   4  XOR B
        case 0xa9: return xor_r_r(&A_REG, &C_REG);       //   4  XOR C
        case 0xaa: return xor_r_r(&A_REG, &D_REG);       //   4  XOR D
        case 0xab: return xor_r_r(&A_REG, &E_REG);       //   4  XOR E
        case 0xac: return xor_r_r(&A_REG, &H_REG);       //   4  XOR H
        case 0xad: return xor_r_r(&A_REG, &L_REG);       //   4  XOR L
        case 0xae: return xor_r_ll(&A_REG, HL_REG);      //   8  XOR (HL)
        case 0xaf: return xor_r_r(&A_REG, &A_REG);       //   4  XOR A
        case 0xb0: return or_r_r(&A_REG, &B_REG);        //   4  OR B
        case 0xb1: return or_r_r(&A_REG, &C_REG);        //   4  OR C
        case 0xb2: return or_r_r(&A_REG, &D_REG);        //   4  OR D
        case 0xb3: return or_r_r(&A_REG, &E_REG);        //   4  OR E
        case 0xb4: return or_r_r(&A_REG, &H_REG);        //   4  OR H
        case 0xb5: return or_r_r(&A_REG, &L_REG);        //   4  OR L
        case 0xb6: return or_r_ll(&A_REG, HL_REG);       //   8  OR (HL)
        case 0xb7: return or_r_r(&A_REG, &A_REG);        //   4  OR A
        case 0xb8: return cp_r_r(&A_REG, &B_REG);        //   4  CP B
        case 0xb9: return cp_r_r(&A_REG, &C_REG);        //   4  CP C
        case 0xba: return cp_r_r(&A_REG, &D_REG);        //   4  CP D
        case 0xbb: return cp_r_r(&A_REG, &E_REG);        //   4  CP E
        case 0xbc: return cp_r_r(&A_REG, &H_REG);        //   4  CP H
        case 0xbd: return cp_r_r(&A_REG, &L_REG);        //   4  CP L
        case 0xbe: return cp_r_ll(&A_REG, HL_REG);       //   8  CP (HL)
        case 0xbf: return cp_r_r(&A_REG, &A_REG);        //   4  CP A
        case 0xc0: return ret_cond(NZ_COND);             //  ??  RET NZ
        case 0xc1: return pop_rr(&BC_REG);               //  12  POP BC
        case 0xc2: return jump_cond_ll(NZ_COND);         //  ??  JP NZ yyxx
        case 0xc3: return jump_nn();                     //  16  JP yyxx
        case 0xc4: return call_cond_nn(NZ_COND);         //  ??  CALL NZ yyxx
        case 0xc5: return push_rr(&BC_REG);              //  16  PUSH BC
        case 0xc6: return add_r_n(&A_REG);               //   8  ADD A, xx
        case 0xc7: return rst_l(0x00);                   //  16  RST 00h
        case 0xc8: return ret_cond(Z_COND);              //  ??  RET Z
        case 0xc9: return ret();                         //  16  RET
        case 0xca: return jump_cond_ll(Z_COND);          //  ??  JP Z yyxx
        case 0xcb: {
            u8 data = fetch_8();

            u8 bit = (data >> 3) & 7;
            u8 *reg = nullptr;

            switch(data & 7) {
            case 0: reg = &B_REG; break;
            case 1: reg = &C_REG; break;
            case 2: reg = &D_REG; break;
            case 3: reg = &E_REG; break;
            case 4: reg = &H_REG; break;
            case 5: reg = &L_REG; break;
            case 6:               break; //don't set this one so we can detect it later
            case 7: reg = &A_REG; break;
            default:              break;
            }

            switch(data >> 6) {
            case 0x00:
                switch(bit) {
                case 0: //RLC
                    if(reg) return rlc_r(reg);
                    else    return rlc_ll(HL_REG);
                case 1: //RRC
                    if(reg) return rrc_r(reg);
                    else    return rrc_ll(HL_REG);
                case 2: //RL
                    if(reg) return rl_r(reg);
                    else    return rl_ll(HL_REG);
                case 3: //RR
                    if(reg) return rr_r(reg);
                    else    return rr_ll(HL_REG);
                case 4: //SLA
                    if(reg) return sla_r(reg);
                    else    return sla_ll(HL_REG);
                case 5: //SRA
                    if(reg) return sra_r(reg);
                    else    return sra_ll(HL_REG);
                case 6: //SWAP
                    if(reg) return swap_r(reg);
                    else    return swap_ll(HL_REG);
                case 7: //SRL
                    if(reg) return srl_r(reg);
                    else    return srl_ll(HL_REG);
                }
                break; //just in case
            case 0x01:
                if(reg) return bit_b_r(bit, reg);
                else    return bit_b_ll(bit, HL_REG);
            case 0x02:
                if(reg) return res_b_r(bit, reg);
                else    return res_b_ll(bit, HL_REG);
            case 0x03:
                if(reg) return set_b_r(bit, reg);
                else    return set_b_ll(bit, HL_REG);
            default:
                return 0;
            }
        }
        break;
        case 0xcc: return call_cond_nn(Z_COND);          //  ??  CALL Z, yyxx
        case 0xcd: return call_nn();                     //  24  CALL yyxx
        case 0xce: return adc_r_n(&A_REG);               //   8  ADC A, xx
        case 0xcf: return rst_l(0x08);                   //  16  RST 08h
        case 0xd0: return ret_cond(NC_COND);             //  ??  RET NC
        case 0xd1: return pop_rr(&DE_REG);               //  12  POP DE
        case 0xd2: return jump_cond_ll(NC_COND);         //  ??  JP NC, yyxx
        case 0xd3: return invalid_op(0xd3);              //  --  ----
        case 0xd4: return call_cond_nn(NC_COND);         //  ??  CALL NC yyxx
        case 0xd5: return push_rr(&DE_REG);              //  12  PUSH DE
        case 0xd6: return sub_r_n(&A_REG);               //   8  SUB xx
        case 0xd7: return rst_l(0x10);                   //  16  RST 10h
        case 0xd8: return ret_cond(C_COND);              //  ??  RET C
        case 0xd9: return reti();                        //  16  RETI
        case 0xda: return jump_cond_ll(C_COND);          //  ??  JP C yyxx
        case 0xdb: return invalid_op(0xdb);              //  --  ----
        case 0xdc: return call_cond_nn(C_COND);          //  ??  CALL C yyxx
        case 0xdd: return invalid_op(0xdd);              //  --  ----
        case 0xde: return sbc_r_n(&A_REG);               //   8  SBC A, xx
        case 0xdf: return rst_l(0x18);                   //  16  RST 18h
        case 0xe0: return load_lln_r(&A_REG);            //  12  LD ($FF00 + xx), A
        case 0xe1: return pop_rr(&HL_REG);               //  12  POP HL
        case 0xe2: return load_llr_r(&C_REG, &A_REG);    //   8  LD ($FF00 + C), A
        case 0xe3: return invalid_op(0xe3);              //  --  ----
        case 0xe4: return invalid_op(0xe4);              //  --  ----
        case 0xe5: return push_rr(&HL_REG);              //  16  PUSH HL
        case 0xe6: return and_r_n(&A_REG);               //   8  AND xx
        case 0xe7: return rst_l(0x20);                   //  16  RST 20h
        case 0xe8: return add_rr_n(&SP_REG);             //   8  ADD  SP, xx
        case 0xe9: return jump_ll(HL_REG);               //   4  JP (HL)
        case 0xea: return load_llnn_r(&A_REG);           //  16  LD (yyxx), A
        case 0xeb: return invalid_op(0xeb);              //  --  ----
        case 0xec: return invalid_op(0xec);              //  --  ----
        case 0xed: return invalid_op(0xed);              //  --  ----
        case 0xee: return xor_r_n(&A_REG);               //   8  XOR xx
        case 0xef: return rst_l(0x28);                   //  16  RST 28h
        case 0xf0: return load_r_lln(&A_REG);            //  12  LD A, ($FF00 + xx)
        case 0xf1: return pop_af(&AF_REG);               //  12  POP AF
        case 0xf2: return load_r_llr(&A_REG, &C_REG);    //   8  LD A, (FF00 + C)
        case 0xf3: return di();                          //   4  DI
        case 0xf4: return invalid_op(0xf4);              //  --  ----
        case 0xf5: return push_rr(&AF_REG);              //  16  PUSH AF
        case 0xf6: return or_r_n(&A_REG);                //   8  OR xx
        case 0xf7: return rst_l(0x30);                   //  16  RST 30h
        case 0xf8: return load_rr_rrn(&HL_REG, &SP_REG); //  12  LD HL, SP + xx
        case 0xf9: return load_rr_rr(&SP_REG, &HL_REG);  //   8  LD SP, HL
        case 0xfa: return load_r_llnn(&A_REG);           //  16  LD A, (yyxx)
        case 0xfb: return ei();                          //   4  EI
        case 0xfc: return invalid_op(0xfc);              //  --  ----
        case 0xfd: return invalid_op(0xfd);              //  --  ----
        case 0xfe: return cp_r_n(&A_REG);                //   8  CP xx
        case 0xff: return rst_l(0x38);                   //  16  RST 38h
    }

    return 0; //to silence the warnings
}

u8 CPU::fetch_8() {
    if(halt_bug) {
        halt_bug = false;
        return io->read(PC_REG);
    } else {
        return io->read(PC_REG++);
    }
}

u16 CPU::fetch_16() {
    u8 q1 = fetch_8();
    u8 q2 = fetch_8();
    return (u16)q2 << 8 | q1;
}

void CPU::stack_push(u16 n) {
    io->write(--SP_REG, n >> 8);
    io->write(--SP_REG, (u8)n);
}

u16  CPU::stack_pop() {
    u8 q1 = io->read(SP_REG++);
    u8 q2 = io->read(SP_REG++);
    return (u16)q2 << 8 | q1;
}

//============
// Nop
//============
u8 CPU::no_op() {
    return 4;
}

//============
// Loads
//============
u8 CPU::load_r_n(u8 *r1) {
    *r1 = fetch_8();
    return 8;
}

u8 CPU::load_r_r(u8 *r1, u8 *r2) {
    *r1 = *r2;
    return 4;
}

u8 CPU::load_r_ll(u8 *r1, u16 loc) {
    *r1 = io->read(loc);
    return 8;
}

u8 CPU::load_rr_nn(u16 *r1) {
    *r1 = fetch_16();
    return 16;
}

u8 CPU::loadi_rr_r(u16 *r1, u8 *r2) {
    io->write(*r1, *r2);
    (*r1)++;
    return 8;
}

u8 CPU::loadi_r_rr(u8 *r1, u16 *r2) {
    *r1 = io->read(*r2);
    (*r2)++;
    return 8;
}

u8 CPU::loadd_rr_r(u16 *r1, u8 *r2) {
    io->write(*r1, *r2);
    (*r1)--;
    return 8;
}

u8 CPU::loadd_r_rr(u8 *r1, u16 *r2) {
    *r1 = io->read(*r2);
    (*r2)--;
    return 8;
}

u8 CPU::load_rr_rr(u16 *r1, u16 *r2) {
    *r1 = *r2;
    return 8;
}

u8 CPU::load_llnn_r(u8 *r1) {
    io->write(fetch_16(), *r1);
    return 16;
}

u8 CPU::load_llnn_rr(u16 *r1) {
    u16 addr = fetch_16();
    io->write(addr++, *r1 & 0xFF);
    io->write(addr, *r1 >> 8);
    return 20;
}

u8 CPU::load_ll_n(u16 loc) {
    io->write(loc, fetch_8());
    return 12;
}

u8 CPU::load_ll_r(u16 loc, u8 *r1 ) {
    io->write(loc, *r1);
    return 8;
}

u8 CPU::load_lln_r(u8 *r1) {
    io->write(0xFF00 + fetch_8(), *r1);
    return 12;
}

u8 CPU::load_llr_r(u8 *r1, u8 *r2) {
    io->write(0xFF00 + *r1, *r2);
    return 8;
}

u8 CPU::load_r_lln(u8 *r1) {
    *r1 = io->read(0xFF00 + fetch_8());
    return 12;
}

u8 CPU::load_r_llr(u8 *r1, u8 *r2) {
    *r1 = io->read(0xFF00 + *r2);
    return 8;
}

u8 CPU::load_rr_rrn(u16 *r1, u16 *r2) {
//CHNZ
    s8 e = (s8)fetch_8(); //signed

    Z_FLAG = 0;
    N_FLAG = 0;

    //stupid signed operand breaks existing carry calculations
    H_FLAG = (*r2 & 0xF) + (e & 0xF) > 0xF;
    C_FLAG = (*r2 & 0xFF) + (e & 0xFF) > 0xFF;

    *r1 = *r2 + e;
    return 12;
}

u8 CPU::load_r_llnn(u8 *r1) {
    *r1 = io->read(fetch_16());
    return 16;
}

//====================
// Increment/Decrement
//====================
u8 CPU::inc_r(u8 *r1) {
//HNZ
    Z_FLAG = !(u8)(*r1 + 1);
    H_FLAG = check_half_carry_8(1, *r1, (u16)*r1 + 1);
    N_FLAG = 0;

    *r1+=1;
    return 4;
}

u8 CPU::dec_r(u8 *r1) {
//HNZ
    Z_FLAG = !(*r1 - 1);
    H_FLAG = check_half_carry_8(1, *r1, *r1 - 1);
    N_FLAG = 1;

    *r1-=1;
    return 4;
}

u8 CPU::inc_rr(u16 *r1) {
    *r1+=1;
    return 8;
}

u8 CPU::dec_rr(u16 *r1) {
    *r1-=1;
    return 8;
}

u8 CPU::inc_ll(u16 loc) {
//HNZ
    u8 r1 = io->read(loc);

    Z_FLAG = !(u8)(r1 + 1);
    H_FLAG = check_half_carry_8(1, r1, (u16)(r1 + 1));
    N_FLAG = 0;

    io->write(loc, r1+1);
    return 12;
}

u8 CPU::dec_ll(u16 loc) {
//HNZ
    u8 r1 = io->read(loc);

    Z_FLAG = !(r1 - 1);
    H_FLAG = check_half_carry_8(r1, 1, (r1 - 1));
    N_FLAG = 1;

    io->write(loc, r1-1);
    return 12;
}

//====================
// Add
//====================
u8 CPU::add_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    Z_FLAG = !(u8)(*r1 + r2);
    H_FLAG = check_half_carry_8(*r1, r2, (u16)*r1 + r2);
    N_FLAG = 0;
    C_FLAG = check_carry_8(*r1, r2, (u16)*r1 + r2);

    *r1 += r2;
    return 8;
}

u8 CPU::add_r_r(u8 *r1, u8 *r2) {
//CHNZ
    Z_FLAG = !(u8)(*r1 + *r2);
    H_FLAG = check_half_carry_8(*r1, *r2, (u16)*r1 + *r2);
    N_FLAG = 0;
    C_FLAG = check_carry_8(*r1, *r2, (u16)*r1 + *r2);
    *r1 += *r2;
    return 4;
}

u8 CPU::add_r_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    Z_FLAG = !(u8)(*r1 + r2);
    H_FLAG = check_half_carry_8(*r1, r2, (u16)*r1 + r2);
    N_FLAG = 0;
    C_FLAG = check_carry_8(*r1, r2, (u16)*r1 + r2);

    *r1 += r2;
    return 8;
}

u8 CPU::add_rr_n(u16 *r1) {
//CHNZ
    s8 r2 = (s8)fetch_8();

    Z_FLAG = 0;
    N_FLAG = 0;

    //stupid signed operand breaks existing carry calculations
    H_FLAG = (*r1 & 0xF) + (r2 & 0xF) > 0xF;
    C_FLAG = (*r1 & 0xFF) + (r2 & 0xFF) > 0xFF;

    *r1 += r2;

    return 16;
}

u8 CPU::add_rr_rr(u16 *r1, u16 *r2) {
//CHN
    H_FLAG = check_half_carry_16(*r1, *r2, (u32)*r1 + *r2);
    N_FLAG = 0;
    C_FLAG = check_carry_16(*r1, *r2, (u32)*r1 + *r2);

    *r1 += *r2;
    return 8;
}

//====================
// Add with Carry
//====================
u8 CPU::adc_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    u8 old_c = C_FLAG; //save the old C_FLAG so we can set the flags first

    Z_FLAG = !(u8)(*r1 + r2 + old_c);
    H_FLAG = check_half_carry_8(*r1, r2, old_c, (u16)*r1 + r2 + old_c);
    N_FLAG = 0;
    C_FLAG = check_carry_8(*r1, r2, old_c, (u16)*r1 + r2 + old_c);

    *r1 += r2 + old_c;
    return 8;
}

u8 CPU::adc_r_r(u8 *r1, u8 *r2) {
//CHNZ
    u8 old_c = C_FLAG; //save the old C_FLAG so we can set the flags first

    Z_FLAG = !(u8)(*r1 + *r2 + old_c);
    H_FLAG = check_half_carry_8(*r1, *r2, old_c, (u16)*r1 + *r2 + old_c);
    N_FLAG = 0;
    C_FLAG = check_carry_8(*r1, *r2, old_c, (u16)*r1 + *r2 + old_c);

    *r1 += *r2 + old_c;
    return 4;
}

u8 CPU::adc_r_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    u8 old_c = C_FLAG; //save the old C_FLAG so we can set the flags first

    Z_FLAG = !(u8)(*r1 + r2 + old_c);
    H_FLAG = check_half_carry_8(*r1, r2, old_c, (u16)*r1 + r2 + old_c);
    N_FLAG = 0;
    C_FLAG = check_carry_8(*r1, r2, old_c, (u16)*r1 + r2 + old_c);

    *r1 += r2 + old_c;
    return 8;
}

//====================
// Subtract
//====================
u8 CPU::sub_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    Z_FLAG = !(u8)(*r1 - r2);
    H_FLAG = check_half_carry_8(*r1, r2, (u16)*r1 - r2);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, r2, (u16)*r1 - r2);

    *r1 -= r2;
    return 8;
}

u8 CPU::sub_r(u8 *r1, u8 *r2) {
//CHNZ

    Z_FLAG = !(u8)(*r1 - *r2);
    H_FLAG = check_half_carry_8(*r1, *r2, (u16)*r1 - *r2);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, *r2, (u16)*r1 - *r2);

    *r1 -= *r2;
    return 4;
}

u8 CPU::sub_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    Z_FLAG = !(u8)(*r1 - r2);
    H_FLAG = check_half_carry_8(*r1, r2, (u16)*r1 - r2);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, r2, (u16)*r1 - r2);

    *r1 -= r2;
    return 8;
}

//====================
// Subtract with Carry
//====================
u8 CPU::sbc_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    u8 old_c = C_FLAG; //save the old C_FLAG so we can set the flags first

    Z_FLAG = !(u8)(*r1 - r2 - old_c);
    H_FLAG = check_half_carry_8(*r1, r2, old_c, *r1 - r2 - old_c);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, r2, old_c, *r1 - r2 - old_c);

    *r1 -= r2;
    *r1 -= old_c;

    return 8;
}

u8 CPU::sbc_r_r(u8 *r1, u8 *r2) {
//CHNZ
    u8 old_c = C_FLAG; //save the old C_FLAG so we can set the flags first

    Z_FLAG = !(u8)(*r1 - *r2 - old_c);
    H_FLAG = check_half_carry_8(*r1, *r2, old_c, *r1 - *r2 - old_c);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, *r2, old_c, *r1 - *r2 - old_c);

    *r1 -= *r2;
    *r1 -= old_c;

    return 4;
}

u8 CPU::sbc_r_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    u8 old_c = C_FLAG; //save the old C_FLAG so we can set the flags first

    Z_FLAG = !(u8)(*r1 - r2 - old_c);
    H_FLAG = check_half_carry_8(*r1, r2, old_c, *r1 - r2 - old_c);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, r2, old_c, *r1 - r2 - old_c);

    *r1 -= r2;
    *r1 -= old_c;

    return 8;
}

//====================
// And
//====================
u8 CPU::and_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    Z_FLAG = !(*r1 & r2);
    H_FLAG = 1;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 &= r2;
    return 8;
}

u8 CPU::and_r_r(u8 *r1, u8 *r2) {
//CHNZ
    Z_FLAG = !(*r1 & *r2);
    H_FLAG = 1;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 &= *r2;
    return 4;
}

u8 CPU::and_r_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    Z_FLAG = !(*r1 & r2);
    H_FLAG = 1;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 &= r2;
    return 8;
}

//====================
// Or
//====================
u8 CPU::or_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    Z_FLAG = !(*r1 | r2);
    H_FLAG = 0;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 |= r2;
    return 8;
}

u8 CPU::or_r_r(u8 *r1, u8 *r2) {
//CHNZ
    Z_FLAG = !(*r1 | *r2);
    H_FLAG = 0;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 |= *r2;
    return 4;
}

u8 CPU::or_r_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    Z_FLAG = !(*r1 | r2);
    H_FLAG = 0;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 |= r2;
    return 8;
}

//====================
// Exclusive Or
//====================
u8 CPU::xor_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    Z_FLAG = !(*r1 ^ r2);
    H_FLAG = 0;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 ^= r2;
    return 8;
}

u8 CPU::xor_r_r(u8 *r1, u8 *r2) {
//CHNZ
    Z_FLAG = !(*r1 ^ *r2);
    H_FLAG = 0;
    N_FLAG = 0;
    C_FLAG = 0;

    *r1 ^= *r2;
    return 4;
}

u8 CPU::xor_r_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    Z_FLAG = !(*r1 ^ r2);
    N_FLAG = 0;
    H_FLAG = 0;
    C_FLAG = 0;

    *r1 ^= r2;
    return 8;
}

//====================
// Compare
//====================
u8 CPU::cp_r_n(u8 *r1) {
//CHNZ
    u8 r2 = fetch_8();

    Z_FLAG = !(*r1-r2);
    H_FLAG = check_half_carry_8(*r1, r2, (u16)*r1-r2);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, r2, (u16)*r1-r2);

    return 8;
}

u8 CPU::cp_r_r(u8 *r1, u8 *r2) {
//CHNZ
    Z_FLAG = !(*r1-*r2);
    H_FLAG = check_half_carry_8(*r1, *r2, (u16)*r1-*r2);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, *r2, (u16)*r1-*r2);

    return 4;
}

u8 CPU::cp_r_ll(u8 *r1, u16 loc) {
//CHNZ
    u8 r2 = io->read(loc);

    Z_FLAG = !(*r1-r2);
    H_FLAG = check_half_carry_8(*r1, r2, (u16)*r1-r2);
    N_FLAG = 1;
    C_FLAG = check_carry_8(*r1, r2, (u16)*r1-r2);

    return 8;
}

//====================
// Push/Pop
//====================
u8 CPU::pop_rr(u16 *r1) {
    *r1 = stack_pop();
    return 12;
}

//special case
u8 CPU::pop_af(u16 *r1) {
    *r1 = stack_pop() & 0xfff0;
    return 12;
}

u8 CPU::push_rr(u16 *r1) {
    stack_push(*r1);
    return 16;
}

//====================
// Rotate
//====================
u8 CPU::rla_r(u8 *r1) {
//CHNZ
    u8 old_c = C_FLAG;

    C_FLAG = Bit::test(*r1, 7);
    H_FLAG = 0;
    N_FLAG = 0;
    Z_FLAG = 0;
    *r1 <<= 1;
    *r1 |= old_c;

    return 4;
}

u8 CPU::rra_r(u8 *r1) {
//CHNZ
    u8 old_c = C_FLAG;

    C_FLAG = *r1 & 1;
    H_FLAG = 0;
    N_FLAG = 0;
    Z_FLAG = 0;
    *r1 >>= 1;
    *r1 |= old_c<<7;

    return 4;
}

u8 CPU::rlca_r(u8 *r1) {
//CHNZ
    C_FLAG = Bit::test(*r1, 7);
    H_FLAG = 0;
    N_FLAG = 0;
    Z_FLAG = 0;
    *r1 <<= 1;
    *r1 |= C_FLAG;

    return 4;
}

u8 CPU::rrca_r(u8 *r1) {
//CHNZ
    C_FLAG = Bit::test(*r1, 0);
    H_FLAG = 0;
    N_FLAG = 0;
    Z_FLAG = 0;
    *r1 >>= 1;
    *r1 |= C_FLAG<<7;

    return 4;
}

u8 CPU::rlc_r(u8 *r1) {
//CHNZ
    C_FLAG = Bit::test(*r1, 7);
    H_FLAG = 0;
    N_FLAG = 0;
    *r1 <<= 1;
    *r1 |= C_FLAG;

    Z_FLAG = !*r1;

    return 8;
}

u8 CPU::rlc_ll(u16 loc) {
//CHNZ
    u8 r = io->read(loc);

    C_FLAG = Bit::test(r, 7);
    H_FLAG = 0;
    N_FLAG = 0;
    r <<= 1;
    r |= C_FLAG;

    Z_FLAG = !r;

    io->write(loc, r);

    return 16;
}

u8 CPU::rrc_r(u8 *r1) {
//CHNZ

    C_FLAG = Bit::test(*r1, 0);
    H_FLAG = 0;
    N_FLAG = 0;
    *r1 >>= 1;
    *r1 |= C_FLAG<<7;

    Z_FLAG = !*r1;

    return 8;
}

u8 CPU::rrc_ll(u16 loc) {
//CHNZ
    u8 r = io->read(loc);

    C_FLAG = Bit::test(r, 0);
    H_FLAG = 0;
    N_FLAG = 0;
    r >>= 1;
    r |= C_FLAG<<7;

    Z_FLAG = !r;

    io->write(loc, r);

    return 4;
}

u8 CPU::rl_r(u8 *r1) {
//CHNZ
    u8 old_c = C_FLAG;

    C_FLAG = Bit::test(*r1, 7);
    H_FLAG = 0;
    N_FLAG = 0;

    *r1 <<= 1;
    *r1 |= old_c;

    Z_FLAG = !*r1;

    return 8;
}

u8 CPU::rl_ll(u16 loc) {
//CHNZ
    u8 r = io->read(loc);
    u8 old_c = C_FLAG;

    C_FLAG = Bit::test(r, 7);
    H_FLAG = 0;
    N_FLAG = 0;

    r <<= 1;
    r |= old_c;

    Z_FLAG = !r;

    io->write(loc, r);

    return 16;
}

u8 CPU::rr_r(u8 *r1) {
//CHNZ
    u8 old_c = C_FLAG;

    C_FLAG = Bit::test(*r1, 0);
    H_FLAG = 0;
    N_FLAG = 0;

    *r1 >>= 1;
    *r1 |= old_c<<7;

    Z_FLAG = !*r1;

    return 8;
}

u8 CPU::rr_ll(u16 loc) {
//CHNZ
    u8 r = io->read(loc);
    u8 old_c = C_FLAG;

    C_FLAG = Bit::test(r, 0);
    H_FLAG = 0;
    N_FLAG = 0;

    r >>= 1;
    r |= old_c<<7;

    Z_FLAG = !r;

    io->write(loc, r);

    return 16;
}

//====================
// Shift
//====================
u8 CPU::sla_r(u8 *r1) {
//CHNZ
    C_FLAG = Bit::test(*r1, 7);
    H_FLAG = 0;
    N_FLAG = 0;

    *r1 <<= 1;

    Z_FLAG = !*r1;

    return 8;
}

u8 CPU::sla_ll(u16 loc) {
//CHNZ
    u8 r = io->read(loc);

    C_FLAG = Bit::test(r, 7);
    H_FLAG = 0;
    N_FLAG = 0;

    r <<= 1;

    Z_FLAG = !r;

    io->write(loc, r);

    return 16;
}

u8 CPU::sra_r(u8 *r1) {
//CHNZ
    u8 a = *r1 & 0x80; //save eight bit for arithmetic shift

    C_FLAG = Bit::test(*r1, 0);
    H_FLAG = 0;
    N_FLAG = 0;

    *r1 >>= 1;
    *r1 |= a; //set eighth bit for arithmetic shift;

    Z_FLAG = !*r1;

    return 8;
}

u8 CPU::sra_ll(u16 loc) {
//CHNZ
    u8 r = io->read(loc);
    u8 a = r & 0x80; //save eight bit for arithmetic shift

    C_FLAG = Bit::test(r, 0);
    H_FLAG = 0;
    N_FLAG = 0;

    r >>= 1;
    r |= a; //set eighth bit for arithmetic shift;

    Z_FLAG = !r;

    io->write(loc, r);

    return 16;
}

u8 CPU::srl_r(u8 *r1) {
//CHNZ

    C_FLAG = Bit::test(*r1, 0);
    H_FLAG = 0;
    N_FLAG = 0;

    *r1 >>= 1;
    *r1 &= 0x7F; // logical shift, force the last bit to zero

    Z_FLAG = !*r1;

    return 8;
}

u8 CPU::srl_ll(u16 loc) {
//CHNZ
    u8 r = io->read(loc);

    C_FLAG = Bit::test(r, 0);
    H_FLAG = 0;
    N_FLAG = 0;

    r >>= 1;
    r &= 0x7F; // logical shift, force the last bit to zero

    Z_FLAG = !r;

    io->write(loc, r);

    return 16;
}

//====================
// Swap
//====================
u8 CPU::swap_r(u8 *r1) {
    u8 t = *r1 >> 4;

    C_FLAG = 0;
    H_FLAG = 0;
    N_FLAG = 0;
    Z_FLAG = !*r1; //swapping a 0x00 will still be 0x00

    *r1 = (*r1 << 4) | t;

    return 8;
}

u8 CPU::swap_ll(u16 loc) {
    u8 r = io->read(loc);
    u8 t = r >> 4;

    C_FLAG = 0;
    H_FLAG = 0;
    N_FLAG = 0;
    Z_FLAG = !r; //swapping a 0x00 will still be 0x00

    r = (r << 4) | t;

    io->write(loc, r);

    return 16;
}

//====================
// Test Bit
//====================
u8 CPU::bit_b_r(u8 bit, u8 *r1) {
//HNZ

    H_FLAG = 1;
    N_FLAG = 0;
    Z_FLAG = !Bit::test(*r1, bit);

    return 8;
}

u8 CPU::bit_b_ll(u8 bit, u16 loc) {
//HNZ
    u8 r1 = io->read(loc);

    H_FLAG = 1;
    N_FLAG = 0;
    Z_FLAG = !Bit::test(r1, bit);

    return 12;
}

//====================
// Reset Bit
//====================
u8 CPU::res_b_r(u8 bit, u8 *reg) {
    Bit::reset(reg, bit);
    return 8;
}

u8 CPU::res_b_ll(u8 bit, u16 loc) {
    u8 r = io->read(loc);

    Bit::reset(&r, bit);
    io->write(loc, r);

    return 16;
}

//====================
// Set Bit
//====================
u8 CPU::set_b_r(u8 bit, u8 *reg) {
    Bit::set(reg, bit);
    return 8;
}

u8 CPU::set_b_ll(u8 bit, u16 loc) {
    u8 r = io->read(loc);

    Bit::set(&r, bit);
    io->write(loc, r);

    return 16;
}

//====================
// Decimal Adjust
//====================
u8 CPU::daa_r(u8 *r1) {
//CHZ

    u8 old_a = *r1;
    u8 old_c =  C_FLAG;

    if(!N_FLAG) { // addition
        // mostly DAA from x86 but gb doesn't set H_FLAG
        if(((*r1 & 0xF) > 9) || H_FLAG) {
            *r1 += 6;
            C_FLAG = 1;
        }

        if((old_a > 0x99) || old_c) {
            *r1 += 0x60;
            C_FLAG = 1;
        } else {
            C_FLAG = 0;
        }
    } else { // subtraction
        //derived from Gameboy Programmers Manual P.122
        if     ( C_FLAG &&  H_FLAG) *r1 += 0x9a;
        else if( C_FLAG && !H_FLAG) *r1 += 0xa0;
        else if(!C_FLAG &&  H_FLAG) *r1 += 0xfa;
    }

    H_FLAG = 0;
    Z_FLAG = !*r1;

    return 4;
}

//====================
// Complement
//====================
u8 CPU::cpl_r(u8 *r1) {
//HN
    H_FLAG = 1;
    N_FLAG = 1;

    *r1 = ~*r1;
    return 4;
}

//====================
// Set/Clear Carry
//====================
u8 CPU::scf() {
    N_FLAG = 0;
    H_FLAG = 0;
    C_FLAG = 1;

    return 4;
}

u8 CPU::ccf() {
    N_FLAG = 0;
    H_FLAG = 0;
    C_FLAG = !C_FLAG;

    return 4;
}

//====================
// Halt/Stop
//====================
u8 CPU::halt() {
    if(!IME && io->cpu_check_interrupts())
        halt_bug = true;
    else
        is_halted = true;

    return 4;
}

u8 CPU::stop() {
    is_stopped = true;
    PC_REG--;
    return 1;
}

//====================
// Enable/Disable Interrupts
//====================
u8 CPU::ei() {
    if(!ei_ime_enable)
        ei_ime_enable = true;
    return 4;
}

u8 CPU::di() {
    IME = 0;
    return 4;
}

//====================
// Reset
//====================
u8 CPU::rst_l(u8 loc) {
    stack_push(PC_REG);
    PC_REG = loc;
    return 16;
}

//====================
// Jump
//====================
u8 CPU::jump_nn() {
    PC_REG = fetch_16();

    return 16;
}

u8 CPU::jump_ll(u16 loc) {
    PC_REG = loc;

    return 4;
}

u8 CPU::jump_cond_ll(bool cond) {
    u16 t = fetch_16(); //need to fetch regardless of the condition;
    if(cond) {
        PC_REG = t;
        return 16;
    }
    return 12;
}

u8 CPU::jump_rel_n() {
    s8 e = fetch_8();
    PC_REG += e;
    return 12;
}

u8 CPU::jump_rel_cond_n(bool cond) {
    s8 e = fetch_8();
    if(cond) {
        PC_REG += e;
        return 12;
    }
    return 8;
}

//====================
// Call
//====================
u8 CPU::call_nn() {
    //PC should be instruction after the call
    u16 new_pc = fetch_16();
    stack_push(PC_REG);
    PC_REG = new_pc;
    return 24;
}

u8 CPU::call_cond_nn(bool cond) {
    //PC should be instruction after the call
    u16 new_pc = fetch_16(); //fetch icrements the PC
    if(cond) {
        stack_push(PC_REG);
        PC_REG = new_pc;
        return 24;
    } else {
        return 12;
    }
}

//====================
// Return
//====================
u8 CPU::ret() {
    PC_REG = stack_pop();
    return 16;
}

u8 CPU::reti() {
    PC_REG = stack_pop();
    IME = 1; //re-enable interrupts
    return 16;
}

u8 CPU::ret_cond(bool cond) {
    if(cond) {
        PC_REG = stack_pop();
        return 20;
    } else {
        return 8;
    }
}

//====================
// Invalid Op
//====================
u8 CPU::invalid_op(u8 op) {
    std::cerr << "Invalid OP" << std::endl;
    PC_REG--; //to make the game freeze
    return 0;
}
