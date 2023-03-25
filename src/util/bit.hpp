#pragma once

#include <cassert>
#include <sstream>
#include <type_traits>
#include <unordered_map>

#if IS_MSVC
#include <intrin.h>
#endif

#include "types/primitives.hpp"
#include "flags.hpp"

// useful definitions
#define bool_to_bit(X) ((X)?1:0)

namespace Bit {
    namespace Mask {
      template<typename T>
      constexpr T bit(uint i) {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);
        return static_cast<T>(1) << i;
      }

      template<typename T>
      constexpr T until(uint i) { return (bit<T>(i)-1); }

      template<typename T>
      constexpr T after(uint i) { return ~(until<T>(i) | bit<T>(i)); }

      template<typename T>
      constexpr T between(uint x, uint y) { return until<T>(x)^until<T>(y); }

      namespace Inclusive {
        template<typename T>
        constexpr T until(uint i) { return (Mask::until<T>(i) | bit<T>(i)); }

        template<typename T>
        constexpr T after(uint i) { return ~(Mask::until<T>(i)); }

        template<typename T>
        constexpr T between(uint x, uint y) { return Mask::between<T>(x,y) | bit<T>(x) | bit<T>(y); }
      }
    }

    __force_inline bool  test(u64 v,  u8 i) { return ((v >> i) & 1); }
    __force_inline void   set(u8  *v, u8 i) {       *v |=  (1 << i); }
    __force_inline void   set(u16 *v, u8 i) {       *v |=  (1U << i); }
    __force_inline void   set(u32 *v, u8 i) {       *v |=  (1UL << i); }
    __force_inline void   set(u64 *v, u8 i) {       *v |=  (1ULL << i); }
    __force_inline u8     set(u8   v, u8 i) { return v |  (1 << i); }
    __force_inline u16    set(u16  v, u8 i) { return v |  (1U << i); }
    __force_inline u32    set(u32  v, u8 i) { return v |  (1UL << i); }
    __force_inline u64    set(u64  v, u8 i) { return v |  (1ULL << i); }
    __force_inline void reset(u8  *v, u8 i) {       *v &= ~(1 << i); }
    __force_inline void reset(u16 *v, u8 i) {       *v &= ~(1U << i); }
    __force_inline void reset(u32 *v, u8 i) {       *v &= ~(1UL << i); }
    __force_inline void reset(u64 *v, u8 i) {       *v &= ~(1ULL << i); }
    __force_inline u8   reset(u8   v, u8 i) { return v &  ~(1 << i); }
    __force_inline u16  reset(u16  v, u8 i) { return v &  ~(1U << i); }
    __force_inline u32  reset(u32  v, u8 i) { return v &  ~(1UL << i); }
    __force_inline u64  reset(u64  v, u8 i) { return v &  ~(1ULL << i); }

    __force_inline
    bool fallen(u64 s_old, u64 s_new, u8 bit) {
        return test(s_old, bit) && !test(s_new, bit);
    }

    __force_inline
    bool risen(u64 s_old, u64 s_new, u8 bit) {
        return !test(s_old, bit) && test(s_new, bit);
    }

    __force_inline
    bool changed(u64 s_old, u64 s_new, u8 bit) {
        return test(s_old, bit) != test(s_new, bit);
    }

    template<typename S, typename U = typename std::make_unsigned<S>::type> __force_inline
    U sign_compress(S reg, u8 sign_bit) {
        auto m = (U)1 << sign_bit;
        return ((as_unsigned(reg) - m) ^ m);
    }

    template<typename U, typename S = typename std::make_signed<U>::type> __force_inline
    S sign_extend(U reg, u8 sign_bit) {
        auto m = (U)1 << sign_bit;
        return as_signed((reg ^ m) - m);
    }

    template<typename T> __force_inline
    u8 count_leading_zeros(T x) {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

        u8 c;
        c = CHAR_BIT * sizeof(T);
        if constexpr (sizeof(T) >= 8) if ((x & 0xffffffff)) { x >>= 32; c -= 32; }
        if constexpr (sizeof(T) >= 4) if ((x & 0xffff))     { x >>= 16; c -= 16; }
        if constexpr (sizeof(T) >= 2) if ((x & 0xff))       { x >>= 8;  c -= 8;  }
                                      if ((x & 0xf))        { x >>= 4;  c -= 4;  }
                                      if ((x & 0x3))        { x >>= 2;  c -= 2;  }
        c -= x & 0x1;
        return c;
    }

    template<typename T> __force_inline
    u8 count_trailing_zeros(T x) {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

        u8 c;
        if (x & 0x1) c = 0;
        else {
            c = 1;
            if constexpr (sizeof(T) >= 8) if ((x & 0xffffffff) == 0) { x >>= 32; c += 32; }
            if constexpr (sizeof(T) >= 4) if ((x & 0xffff) == 0)     { x >>= 16; c += 16; }
            if constexpr (sizeof(T) >= 2) if ((x & 0xff) == 0)       { x >>= 8;  c += 8;  }
                                          if ((x & 0xf) == 0)        { x >>= 4;  c += 4;  }
                                          if ((x & 0x3) == 0)        { x >>= 2;  c += 2;  }
            c -= x & 0x1;
        }
        return c;
    }

    __force_inline u32 byte_swap(u32 x) {
        return
#if IS_GCC
            __builtin_bswap32(x);
#elif IS_MSVC
            _byteswap_ulong(x);
#endif
    }

    __force_inline u64 byte_swap(u64 x) {
        return
#if IS_GCC
            __builtin_bswap64(x);
#elif IS_MSVC
            _byteswap_uint64(x);
#endif
    }

    template<typename T>
    struct shift_with_carry {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);
        static constexpr u8 bit_count = CHAR_BIT * sizeof(T);

        static __force_inline
        T shift_left(T in, u8 shift_amount, bool carry_in, bool *carry_out) {
            if(carry_out != nullptr) {
                if(shift_amount) *carry_out = Bit::test(in, bit_count - shift_amount);
                else             *carry_out = carry_in;
            }
            return in << shift_amount;
        }

        static __force_inline
        T shift_right(T in, u8 shift_amount, bool *carry_out) {
            if(carry_out != nullptr) {
                if(shift_amount) *carry_out = Bit::test(in, shift_amount - 1);
                else             *carry_out = Bit::test(in, bit_count - 1);
            }
            return in >> shift_amount;
        }

        static __force_inline
        T arithmetic_shift_right(T in, u8 shift_amount, bool *carry_out) {
            if(carry_out != nullptr) {
                if(shift_amount) *carry_out = Bit::test(in, shift_amount - 1);
                else             *carry_out = Bit::test(in, bit_count - 1);
            }
            return sign_extend(in >> shift_amount, bit_count - shift_amount);
        }

        static __force_inline
        T rotate_right(T in, u8 rotate_amt, bool *carry_out) {
            // if we hit this at any point, then we'll need a safer implementation
            assert (rotate_amt < bit_count);
            if(carry_out != nullptr) {
                if(rotate_amt) *carry_out = Bit::test(in, rotate_amt - 1);
                else           *carry_out = Bit::test(in, bit_count - 1);
            }

            return (in<<rotate_amt) | (in>>(-rotate_amt & (bit_count - 1)));
        }
    };


};