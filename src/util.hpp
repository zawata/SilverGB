#include <iostream>
#include <string>

#ifndef UTIL_HPP
#define UTIL_HPP

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

class Utility_Functions {
public:
    static bool byteCompare(const u8 *a, const u8 *b, u64 l);
    static std::string itoh(u64 i, u8 zp = 0);
};

#endif