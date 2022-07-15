#pragma once

#ifdef __cplusplus
extern "C" {

  #include <cstdint>
#else
  #include <stdint.h>
#endif

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#ifdef __cplusplus
}
#endif

#define make_int_literal_suffix(T)                                                                                     \
  inline constexpr T operator"" _##T(unsigned long long value) {                                                       \
    return static_cast<T>(value);                                                                                      \
  }

#ifdef __cplusplus
// clang-format off
make_int_literal_suffix(u8)
make_int_literal_suffix(u16)
make_int_literal_suffix(u32)
make_int_literal_suffix(u64)
make_int_literal_suffix(s8)
make_int_literal_suffix(s16)
make_int_literal_suffix(s32)
make_int_literal_suffix(s64)
// clang-format on
#endif