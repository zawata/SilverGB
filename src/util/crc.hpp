#pragma once

#include "util/ints.hpp"

static struct __CRC {
    u32 __table[256];
    __CRC() {
        u32 polynomial = 0xEDB88320;
        for (u32 i = 0; i < 256; i++) {
            u32 c = i;
            for (u32 j = 0; j < 8; j++)
                if (c & 1)
                    c = polynomial ^ (c >> 1);
                else
                    c >>= 1;
            __table[i] = c;
        }
    }

    u32 update(u32 initial, const void* buf, u32 len) {
        u32 c = initial ^ 0xFFFFFFFF;
        const u8* u = static_cast<const u8*>(buf);
        for (u32 i = 0; i < len; ++i)
            c = __table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
        return c ^ 0xFFFFFFFF;
    }

} crc;