#include <iostream>
#include <string>

#ifndef UTIL_HPP
#define UTIL_HPP

#ifndef INTS_DEF
#define INTS_DEF
 #define s8    signed char
 #define s16   signed short
 #define s32   signed long
 #define s64   signed long long
 #define u8  unsigned char
 #define u16 unsigned short
 #define u32 unsigned long
 #define u64 unsigned long long
#endif

class Utility_Functions {
public:
    static bool byteCompare(const u8 *a, const u8 *b, u64 l);
    static std::string itoh(u64 i, u8 zp = 0);
};

#endif