#pragma once

#include "ints.hpp"

#include <iostream>
#include <string>

static inline bool byteCompare(const u8 *a, const u8 *b, u64 l) { while(l--) if(*a++ != *b++) break; return l == 0; }
static inline bool bounded(u8 x, u8 y, u8 z)                    { return (x >= y) && (x <= z); }

static inline std::string itoh(u64 i, u8 zp = 0, bool caps = false) {
    char buf[17];
    if(!caps) {
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
    }
    else {
        switch(zp) {
        case 0:
            snprintf(buf, 17, "%llX", i);
            break;
        case 2:
            snprintf(buf, 17, "%02hhX", (u8)i);
            break;
        case 4:
            snprintf(buf, 17, "%04hX", (u16)i);
            break;
        case 8:
            snprintf(buf, 17, "%08lX", (u32)i);
            break;
        case 16:
            snprintf(buf, 17, "%016llX", (u64)i);
            break;
        }
    }
    return std::string(buf);
}

static inline u64 htoi(const char *h) {
    return strtoull(h, nullptr, 16);
}