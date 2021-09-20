#pragma once

#include "ints.hpp"

#include <cstring>
#include <iostream>
#include <string>

#define as_hex(x) std::hex << (s64)(x) << std::dec

static inline bool byteCompare(const u8 *a, const u8 *b, u64 l) { return ::memcmp(a, b, l) == 0; }

/**
 * a <= value <= b
 **/
template<typename T>
static inline bool bounded(T value, T a, T b) {
    static_assert(std::is_arithmetic<T>::value);
    return (value >= a) && (value <= b);
}

//TODO: this has become unruly and terrible. do it again
static inline std::string itoh(u64 i, u8 zp = 0, bool caps = false) {
    char buf[17] = { 0 };

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
            snprintf(buf, 17, "%08x", (u32)i);
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
            snprintf(buf, 17, "%08X", (u32)i);
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

static void hexDump (const char * desc, const void * addr, const int len) {
    int i;
    unsigned char buff[17];
    const unsigned char * pc = (const unsigned char *)addr;

    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    else if (len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf ("  %s\n", buff);
            printf ("  %04x ", i);
        }
        printf (" %02x", pc[i]);

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) 
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    printf ("  %s\n", buff);
}