#pragma once

#include "ints.hpp"

//TODO: make this static
struct {
    u8    test(int reg,  u8 i) { return (reg >> i) & 1; }

    void   set(u8  *reg, u8 i) { *reg |= 1 << i; }
    void   set(u16 *reg, u8 i) { *reg |= 1U << i; }
    void   set(u32 *reg, u8 i) { *reg |= 1UL << i; }
    void   set(u64 *reg, u8 i) { *reg |= 1ULL << i; }

    void reset(u8  *reg, u8 i) { *reg &= ~(1 << i); }
    void reset(u16 *reg, u8 i) { *reg &= ~(1U << i); }
    void reset(u32 *reg, u8 i) { *reg &= ~(1UL << i); }
    void reset(u64 *reg, u8 i) { *reg &= ~(1ULL << i); }
} Bit;