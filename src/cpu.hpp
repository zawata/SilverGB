#include "mem.hpp"
#include "util.hpp"

#include <iostream>

#ifndef CPU_HPP
#define CPU_HPP

#define CY_FLAG 0x10
#define  H_FLAG 0x20
#define  N_FLAG 0x40
#define ZF_FLAG 0x80

class CPU {
public:
    CPU(Memory_Map *m);
    ~CPU();

    void cycle();

private:
    //Class members:
    Memory_Map *mem;

    //registers:
    union {
        struct {
            union {
                struct {
                    u8 __u : 4; //unused
                    u8 cy  : 1; //carry
                    u8 h   : 1;
                    u8 n   : 1;
                    u8 zf  : 1;
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


    u8  fetch_8();
    u16 fetch_16();

    void store(u16 offset, u8 data);

    void load_8(u8 *dest, u8 data);
    void load_16(u16 *dest, u16 data);

    void ex_or(u8);

    //op generalizations
    void jump(u16 offset);
    void jump_cond(u8 flag, u16 offset);

    void jump_return(u16 offset);
    void jump_return_cond(u8 flag, u16 offset);

};

#endif