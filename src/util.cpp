#include "util.hpp"

bool Utility_Functions::byteCompare(const u8 *a, const u8 *b, u64 l) {
    while(l--) if(*a++ != *b++) break;
    return l == 0;
}

//Inclusive
bool Utility_Functions::bounded(u8 x, u8 y, u8 z) {
    return (x >= y) && (x <= z);
}

std::string Utility_Functions::itoh(u64 i, u8 zp) {
    char buf[17];
    switch(zp){
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