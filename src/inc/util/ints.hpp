#pragma once

#include "util/flags.hpp"

#ifdef __cplusplus
extern "C" {

#include <cstdint>
#else
#include <stdint.h>
#endif

typedef  int8_t  s8;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#ifdef __cplusplus
}

inline constexpr u8  operator "" _u8 (unsigned long long value) { return static_cast<u8>(value); }
inline constexpr u16 operator "" _u16(unsigned long long value) { return static_cast<u16>(value); }
inline constexpr u32 operator "" _u32(unsigned long long value) { return static_cast<u32>(value); }
inline constexpr u64 operator "" _u64(unsigned long long value) { return static_cast<u64>(value); }

inline constexpr s8  operator "" _s8 (unsigned long long value) { return static_cast<s8>(value); }
inline constexpr s16 operator "" _s16(unsigned long long value) { return static_cast<s16>(value); }
inline constexpr s32 operator "" _s32(unsigned long long value) { return static_cast<s32>(value); }
inline constexpr s64 operator "" _s64(unsigned long long value) { return static_cast<s64>(value); }

__force_inline s32 as_signed(u32 v) { return *reinterpret_cast<s32 *>(&v); }
__force_inline u32 as_unsigned(s32 v) { return *reinterpret_cast<u32 *>(&v); }

#endif