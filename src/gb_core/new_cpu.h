#pragma once

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