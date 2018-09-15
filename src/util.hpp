#pragma once

#include <iostream>
#include <string>

#ifndef INTS_DEF
#define INTS_DEF
typedef   signed char      s8 ;
typedef   signed short     s16;
typedef   signed long      s32;
typedef   signed long long s64;
typedef unsigned char      u8 ;
typedef unsigned short     u16;
typedef unsigned long      u32;
typedef unsigned long long u64;
#endif

static inline bool byteCompare(const u8 *a, const u8 *b, u64 l) { while(l--) if(*a++ != *b++) break; return l == 0; }
static inline bool bounded(u8 x, u8 y, u8 z)                    { return (x >= y) && (x <= z); }

static inline std::string itoh(u64 i, u8 zp = 0) {
    char buf[17];
    switch(zp) {
    case 0:
        snprintf(buf, 17, "%llx", i);
        break;
    case 2:
        snprintf(buf, 17, "%02hhx", (u8)i);
        break;
    case 4:
        snprintf(buf, 17, "%04hx", (u16)i);
        break;
    case 8:
        snprintf(buf, 17, "%08lx", (u32)i);
        break;
    case 16:
        snprintf(buf, 17, "%016llx", (u64)i);
        break;
    }
    return std::string(buf);
}

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