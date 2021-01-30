#pragma once

#include "ints.hpp"

static struct {
    inline static bool  test(u64 reg,  u8 i) { return (reg >> i) & 1; }

    inline static void   set(u8  *reg, u8 i) { *reg |=  (1 << i); }
    inline static void   set(u16 *reg, u8 i) { *reg |=  (1U << i); }
    inline static void   set(u32 *reg, u8 i) { *reg |=  (1UL << i); }
    inline static void   set(u64 *reg, u8 i) { *reg |=  (1ULL << i); }
    inline static void reset(u8  *reg, u8 i) { *reg &= ~(1 << i); }
    inline static void reset(u16 *reg, u8 i) { *reg &= ~(1U << i); }
    inline static void reset(u32 *reg, u8 i) { *reg &= ~(1UL << i); }
    inline static void reset(u64 *reg, u8 i) { *reg &= ~(1ULL << i); }

    inline static bool fallen(u64 s_old, u64 s_new, u8 bit) {
        return test(s_old, bit) && !test(s_new, bit);
    }

    inline static bool risen(u64 s_old, u64 s_new, u8 bit) {
        return !test(s_old, bit) && test(s_new, bit);
    }

    inline static bool changed(u64 s_old, u64 s_new, u8 bit) {
        return test(s_old, bit) != test(s_new, bit);
    }
} Bit;