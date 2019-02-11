#pragma once

#include "ints.hpp"

#include <iostream>
#include <string>

static inline bool byteCompare(const u8 *a, const u8 *b, u64 l) { while(l--) if(*a++ != *b++) break; return l == 0; }
static inline bool bounded(u8 x, u8 y, u8 z)                    { return (x >= y) && (x <= z); }

//TODO: this has become unruly and terrible. do it again
static inline std::string itoh(u64 i, u8 zp = 0, bool caps = false) {
    char buf[17];
    if(!caps) {
        switch(zp) {
        case 0:
#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
            snprintf(buf, 17, "%lx", i);
#else
            snprintf(buf, 17, "%llx", i);
#endif
            break;
        case 2:
            snprintf(buf, 17, "%02hhx", (u8)i);
            break;
        case 4:
            snprintf(buf, 17, "%04hx", (u16)i);
            break;
        case 8:
#if __SIZEOF_LONG__ == __SIZE_INT__
            snprintf(buf, 17, "%08lx", (u32)i);
#else
            snprintf(buf, 17, "%08x", (u32)i);
#endif
            break;
        case 16:
#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
            snprintf(buf, 17, "%016lx", (u64)i);
#else
            snprintf(buf, 17, "%016llx", (u64)i);
#endif
            break;
        }
    }
    else {
        switch(zp) {
        case 0:
#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
            snprintf(buf, 17, "%lX", i);
#else
            snprintf(buf, 17, "%llX", i);
#endif
            break;
        case 2:
            snprintf(buf, 17, "%02hhX", (u8)i);
            break;
        case 4:
            snprintf(buf, 17, "%04hX", (u16)i);
            break;
        case 8:
#if __SIZEOF_LONG__ == __SIZE_INT__
            snprintf(buf, 17, "%08lX", (u32)i);
#else
            snprintf(buf, 17, "%08X", (u32)i);
#endif
            break;
        case 16:
#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
            snprintf(buf, 17, "%016lX", (u64)i);
#else
            snprintf(buf, 17, "%016llX", (u64)i);
#endif
            break;
        }
    }
    return std::string(buf);
}

static inline u64 htoi(const char *h) {
    return strtoull(h, nullptr, 16);
}