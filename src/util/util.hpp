#pragma once

#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>

#include "flags.hpp"
#include "types/primitives.hpp"
#include "types/vector.hpp"

#define as_hex(x)       std::hex << (s64)(x) << std::dec

#define array_length(x) sizeof(x) / sizeof(*x)

__force_inline bool byteCompare(const u8 *a, const u8 *b, u64 l) { return ::memcmp(a, b, l) == 0; }

/**
 * a <= value <= b
 **/
template<typename T, typename L>
__force_inline bool bounded(T value, L a, L b) {
    static_assert(std::is_arithmetic<T>::value && (std::is_arithmetic<L>::value || std::is_enum<L>::value), "");
    return (value >= a) && (value <= b);
}

template<typename T>
__force_inline std::string itoh(T i, u8 zp = 0, bool caps = false, bool include_hex_delim = true) {
    static_assert(std::is_integral<T>::value, "");

    std::stringstream stream;
    if(caps) {
        stream << std::uppercase;
    }

    stream << (include_hex_delim ? "0x" : "") << std::setfill('0') << std::setw(zp ? zp : sizeof(T) * 2) << std::hex
           << (s64)i;
    return stream.str();
}

__force_inline u64  htoi(const char *h) { return strtoull(h, nullptr, 16); }

__force_inline void hexDump(const char *desc, const void *addr, const int len) {
    int                  i;
    unsigned char        buff[17];
    const unsigned char *pc = (const unsigned char *)addr;

    if(desc != NULL) {
        printf("%s:\n", desc);
    }

    if(len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    } else if(len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    for(i = 0; i < len; i++) {
        if((i % 16) == 0) {
            if(i != 0) {
                printf("  %s\n", buff);
            }
            printf("  %04x ", i);
        }
        printf(" %02x", pc[i]);

        if((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }
        buff[(i % 16) + 1] = '\0';
    }

    while((i % 16) != 0) {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
}
