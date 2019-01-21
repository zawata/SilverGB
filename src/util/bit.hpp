#pragma once

#include "ints.hpp"

static struct {
    static u8    test(int reg,  u8 i) { return (reg >> i) & 1; }

    static void   set(u8  *reg, u8 i) { *reg |= 1 << i; }
    static void   set(u16 *reg, u8 i) { *reg |= 1U << i; }
    static void   set(u32 *reg, u8 i) { *reg |= 1UL << i; }
    static void   set(u64 *reg, u8 i) { *reg |= 1ULL << i; }

    static void reset(u8  *reg, u8 i) { *reg &= ~(1 << i); }
    static void reset(u16 *reg, u8 i) { *reg &= ~(1U << i); }
    static void reset(u32 *reg, u8 i) { *reg &= ~(1UL << i); }
    static void reset(u64 *reg, u8 i) { *reg &= ~(1ULL << i); }
} Bit;