#pragma once

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

typedef u64      size_t;

#ifdef __cplusplus
}

inline std::uint8_t  operator "" _u8 (unsigned long long value) { return static_cast<std::uint8_t>(value); }
inline std::uint16_t operator "" _u16(unsigned long long value) { return static_cast<std::uint16_t>(value); }
inline std::uint16_t operator "" _u32(unsigned long long value) { return static_cast<std::uint16_t>(value); }
inline std::uint64_t operator "" _u64(unsigned long long value) { return static_cast<std::uint64_t>(value); }

inline std::int8_t  operator "" _s8 (unsigned long long value) { return static_cast<std::int8_t>(value); }
inline std::int16_t operator "" _s16(unsigned long long value) { return static_cast<std::int16_t>(value); }
inline std::int32_t operator "" _s32(unsigned long long value) { return static_cast<std::int32_t>(value); }
inline std::int64_t operator "" _s64(unsigned long long value) { return static_cast<std::int64_t>(value); }

#endif