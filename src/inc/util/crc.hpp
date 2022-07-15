#pragma once

#include <zlib.h>

#include "util/ints.hpp"

struct crc {
    static u32 begin() { return crc32(0, 0, 0); }

    static u32 update(u32 initial, const void *buf, u32 len) { return crc32(initial, (u8 *)buf, len); }
};
