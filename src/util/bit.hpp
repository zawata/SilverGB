#pragma once

#include <cassert>
#include <sstream>
#include <type_traits>
#include <unordered_map>


#if defined(__cpp_lib_bitops)
  #include <bit>
  #define CPP_BIT 1
#else
  #define CPP_BIT 0
#endif

#if IS_MSVC
  #include <intrin.h>
#endif

#include "types/primitives.hpp"
#include "flags.hpp"

namespace Bit {
  /*
   * Forward Declarations
   */
  template<typename T>
  inline uint ctz(T v);

  /*
   * Types
   */
  template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = true>
  constexpr uint count = CHAR_BIT * sizeof(T);

  /*
   * Construct Bit Masks
   */
  namespace Mask {
    /**
     * Set bit `i` in mask of type T
     * @tparam T Integral, Unsigned
     * @param i bit index
     * @return Bit Mask of type T
     */
    template<typename T>
    constexpr T bit(uint i) {
      static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);
      return static_cast<T>(1) << i;
    }

    // template<typename T>
    // T __force_inline range(uint a) {
    //   return bit<T>(a);
    // }

    // template<typename T, typename... Targs>
    // T __force_inline range(uint a, Targs... args) {
    //   return bit<T>(a) | range<T, Targs...>(std::forward(args...));
    // }

    /**
     * Set all bits until index `i` in mask of type T
     * @tparam T Integral, Unsigned
     * @param i bit index
     * @return Bit Mask of type T
     */
    template<typename T>
    __force_inline T until(uint i) {
      return (bit<T>(i) - 1);
    }

    /**
     * Set all bits until index `i`(inclusive) in mask of type T
     * @tparam T Integral, Unsigned
     * @param i bit index
     * @return Bit Mask of type T
     */
    template<typename T>
    __force_inline T until_inc(uint i) {
      return (until<T>(i) | bit<T>(i));
    }

    /**
     * Set all bits after index `i` in mask of type T
     * @tparam T Integral, Unsigned
     * @param i bit index
     * @return Bit Mask of type T
     */
    template<typename T>
    __force_inline T after(uint i) {
      return ~(until<T>(i) | bit<T>(i));
    }

    /**
     * Set all bits after index `i`(inclusive) in mask of type T
     * @tparam T Integral, Unsigned
     * @param i bit index
     * @return Bit Mask of type T
     */
    template<typename T>
    __force_inline T after_inc(uint i) {
      return ~(until<T>(i));
    }

    /**
     * Set all bits in range `x`(inclusive) to `y` in mask of type T
     * @tparam T Integral, Unsigned
     * @param i bit index
     * @return Bit Mask of type T
     */
    template<typename T>
    __force_inline T between(uint x, uint y) {
      return until<T>(x) ^ until<T>(y);
    }

    /**
     * Set all bits in range `x`(inclusive) to `y`(inclusive) in mask of type T
     * @tparam T Integral, Unsigned
     * @param i bit index
     * @return Bit Mask of type T
     */
    template<typename T>
    __force_inline T between_inc(uint x, uint y) {
      return between<T>(x, y) | bit<T>(x) | bit<T>(y);
    }
  }// namespace Mask

  /**
   * Get bits masked by `m` from `v`
   */
  template<typename T>
  __force_inline T mask(T v, T m) {
    return v & m;
  }

  /**
   * Get bit range masked by `m` from `v` shifted to
   */
  template<typename T>
  __force_inline T range(T v, T m) {
    return mask(v, m) >> ctz<T>(m);
  }

  /**
   * explicitly convert a boolean value to a bit
   * @param b value
   * @return single bit value
   */
  __force_inline uint from_bool(bool b) {
    return b ? 1 : 0;
  }

  /**
   * Test bit `i` in `v`
   * @tparam T Integral, Unsigned
   * @param v value to test
   * @param i bit index
   * @return T
   */
  template<typename T>
  __force_inline bool test(T v, uint i) {
    return (v & Mask::bit<T>(i)) != 0;
  }

  /**
   * Set bit `i` in `v`
   * @tparam T Integral, Unsigned
   * @param v value to modify
   * @param i bit index
   */
  template<typename T>
  __force_inline void set(T *v, uint i) {
    *v |= Mask::bit<T>(i);
  }

  /**
   * Set bit `i` in `v`
   * @tparam T Integral, Unsigned
   * @param v value to modify
   * @param i bit index
   * @return T
   */
  template<typename T>
  __force_inline T set(T v, uint i) {
    return v | Mask::bit<T>(i);
  }

  /**
   * set bit `i` in `v` conditionally
   * @tparam T Integral, Unsigned
   * @param v ptr to value
   * @param i bit index
   * @param c conditional value
   */
  template<typename T>
  __force_inline void set_cond(T *v, uint i, bool c) {
    if (c) {
      set(v, i);
    }
  }

  /**
   * set bit `i` in `v` conditionally
   * @tparam T Integral, Unsigned
   * @param v value
   * @param i bit index
   * @param c conditional value
   * @return T
   */
  template<typename T>
  __force_inline T set_cond(T v, uint i, bool c) {
    if (c) {
      return set(v, i);
    }
  }

  /**
   * Unset bit `i` in `v`
   * @tparam T Integral, Unsigned
   * @param v ptr to value to modify
   * @param i bit index
   * @return T
   */
  template<typename T>
  __force_inline void reset(T *v, uint i) {
    *v &= ~Mask::bit<T>(i);
  }

  /**
   * Unset bit `i` in `v`
   * @tparam T Integral, Unsigned
   * @param v value to modify
   * @param i bit index
   * @return T
   */
  template<typename T>
  __force_inline T reset(T v, uint i) {
    return v & ~Mask::bit<T>(i);
  }

  /**
   * Toggle bit 'i' in ptr to value 'v'
   * @tparam T Integral, Unsigned
   * @param v value
   * @param i bit index
   */
  template<typename T>
  __force_inline void toggle(T *v, uint i) {
    *v ^= Mask::bit<T>(i);
  }

  /**
   * Toggle bit 'i' in value 'v'
   * @tparam T Integral, Unsigned
   * @param v value
   * @param i bit index
   * @return T
   */
  template<typename T>
  __force_inline T toggle(T v, uint i) {
    return v ^ Mask::bit<T>(i);
  }

  /**
   * Test if bit `i` has fallen from `s_old` to `s_new`
   * @tparam T Integral, Unsigned
   * @param s_old old value
   * @param s_new new value
   * @param i bit index
   * @return bool
   */
  template<typename T>
  __force_inline bool fallen(T s_old, T s_new, uint i) {
    return test<T>(s_old, i) && !test<T>(s_new, i);
  }

  /**
   * Test if bit `i` has risen from `s_old` to `s_new`
   * @tparam T Integral, Unsigned
   * @param s_old old value
   * @param s_new new value
   * @param i bit index
   * @return bool
   */
  template<typename T>
  __force_inline bool risen(T s_old, T s_new, uint i) {
    return !test<T>(s_old, i) && test<T>(s_new, i);
  }

  /**
   * Test if bit `i` has changed from `s_old` to `s_new`
   * @tparam T Integral, Unsigned
   * @param s_old old value
   * @param s_new new value
   * @param i bit index
   * @return bool
   */
  template<typename T>
  __force_inline bool changed(T s_old, T s_new, uint i) {
    return test<T>(s_old, i) != test<T>(s_new, i);
  }

  /**
   * Move sign bit right on value `V`
   * @tparam S Signed, Integral
   * @tparam U Unsigned<S>
   * @param v value
   * @param i sign bit
   * @return sign-compressed `v`
   */
  template<typename S, typename U = typename std::make_unsigned<S>::type>
  __force_inline U sign_compress(S v, uint i) {
    return (as_unsigned(v) - Bit::Mask::bit<U>(i)) ^ Bit::Mask::bit<U>(i);
  }

  /**
   * Move sign bit left on value `V`
   * @tparam U Unsigned, Integral
   * @tparam S Signed<U>
   * @param v value
   * @param i sign bit
   * @return sign-extended `v`
   */
  template<typename U, typename S = typename std::make_signed<U>::type>
  __force_inline S sign_extend(U reg, uint i) {
    return as_signed((reg ^ Bit::Mask::bit<U>(i)) - Bit::Mask::bit<U>(i));
  }

  namespace {
    template<typename T>
    inline uint _backup_clz(T v) {
      static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

      uint c = Bit::count<T>;
      if constexpr (sizeof(T) >= 8) {
        if ((v & 0xffffffff)) {
          v >>= 32;
          c -= 32;
        }
      }

      if constexpr (sizeof(T) >= 4) {
        if ((v & 0xffff)) {
          v >>= 16;
          c -= 16;
        }
      }

      if constexpr (sizeof(T) >= 2) {
        if ((v & 0xff)) {
          v >>= 8;
          c -= 8;
        }
      }

      if ((v & 0xf)) {
        v >>= 4;
        c -= 4;
      }

      if ((v & 0x3)) {
        v >>= 2;
        c -= 2;
      }

      c -= v & 0x1;
      return c;
    }

    template<typename T>
    inline uint _backup_ctz(T v) {
      uint c;
      if (v & 0x1) {
        c = 0;
      } else {
        c = 1;
        if constexpr (sizeof(T) >= 8) {
          if ((v & 0xffffffff) == 0) {
            v >>= 32;
            c += 32;
          }
        }

        if constexpr (sizeof(T) >= 4) {
          if ((v & 0xffff) == 0) {
            v >>= 16;
            c += 16;
          }
        }

        if constexpr (sizeof(T) >= 2) {
          if ((v & 0xff) == 0) {
            v >>= 8;
            c += 8;
          }
        }

        if ((v & 0xf) == 0) {
          v >>= 4;
          c += 4;
        }

        if ((v & 0x3) == 0) {
          v >>= 2;
          c += 2;
        }

        c -= v & 0x1;
      }
      return c;
    }
  }// namespace

  /**
   * count of unset bits to the left on value `v`
   * @tparam T Integral Unsigned
   * @param v value
   * @return count of unset bits to the left
   */
  template<typename T>
  inline uint clz(T v) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

    if (v == 0) {
      return 0;
    }

#if CPP_BIT
    return std::countl_zero(v);
#elif IS_GCC
    if constexpr (Bit::count<T> <= 32) {
      constexpr uint width = 32 - Bit::count<T>;
      return __builtin_clzl(v) - width;
    } else if constexpr (Bit::count<T> <= 64) {
      return __builtin_clzll(v);
    } else {
      unreachable();
    }
#elif IS_MSVC
    if constexpr (bit_cnt<T> <= 32) {
      u32 tz = 0;
      constexpr uint width = 32 - bit_cnt<T>;
      _BitScanForward(&tz, v);
      return tz - width;
    } else if constexpr (bit_cnt<T> <= 64) {
      u64 tz = 0;
      _BitScanForward64(&tz, v);
      return tz;
    } else {
      unreachable();
    }
#else
    return _backup_ctz<T>(v);
#endif
  }

  /**
   * count of unset bits to the right on value `v`
   * @tparam T Integral Unsigned
   * @param v value
   * @return count of trailing zeros
   */
  template<typename T>
  inline uint ctz(T v) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

    if (v == 0) {
      return 0;
    }

#if CPP_BIT
    return std::countr_zero(v);
#elif IS_GCC
    if constexpr (Bit::count<T> <= 32) {
      uint width = 32 - Bit::count<T>;
      return __builtin_ctzl(v) - width;
    } else if constexpr (Bit::count<T> <= 64) {
      return __builtin_ctzll(v);
    } else {
      unreachable();
    }
#elif IS_MSVC
    if constexpr (Bit::count<T> <= 32) {
      u32 tz = 0;
      uint width = 32 - Bit::count<T>;
      _BitScanReverse(&tz, v);
      return width - tz;
    } else if constexpr (Bit::count<T> <= 64) {
      u64 tz = 0;
      _BitScanReverse64(&tz, v);
      return 64 - tz;
    } else {
      unreachable();
    }
#else
    return _backup_clz<T>(v);
#endif
  }

  /**
   * toggle byte order of value 'v'
   * @tparam T Unsigned, Integral, 32 or 64 bits
   * @param v value
   * @return `v`, byte-swapped
   */
  template<typename T>
  __force_inline T byte_swap(T v) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value && (sizeof(T) == 8 || sizeof(T) == 4));
    if constexpr (sizeof(T) == 8) {
#if IS_GCC
      return __builtin_bswap64(v);
#elif IS_MSVC
      return _byteswap_uint64(v);
#endif
    } else if constexpr (sizeof(T) == 4) {
#if IS_GCC
      return __builtin_bswap32(v);
#elif IS_MSVC
      return _byteswap_ulong(v);
#endif
    } else {
      unreachable();
    }
  }

  /**
   * Shifts preserving carried bits
   */
  namespace ShiftWithCarry {
    /**
     * Logical Left Shift value `v` with carry
     * @param v value to shift
     * @param amt amount to shift
     * @param c_in input carry flag
     * @param c_out output carry flag or nullptr
     * @return `v` shifted by amt
     */
    template<typename T>
    static __force_inline T logical_left(T v, uint amt, bool c_in, bool *c_out) {
      static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

      static constexpr u8 bit_count = Bit::count<T>;

      if (c_out != nullptr) {
        if (amt) {
          *c_out = Bit::test(v, bit_count - amt);
        } else {
          *c_out = c_in;
        }
      }

      return v << amt;
    }

    /**
     * Logical Right Shift value `v` with carry
     * @param v value to shift
     * @param amt amount to shift
     * @param c_out output carry flag or nullptr
     * @return `v` shifted by amt
     */
    template<typename T>
    static __force_inline T logical_right(T in, uint amt, bool *c_out) {
      static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

      static constexpr u8 bit_count = Bit::count<T>;

      if (c_out != nullptr) {
        if (amt) {
          *c_out = Bit::test(in, amt - 1);
        } else {
          *c_out = Bit::test(in, bit_count - 1);
        }
      }

      return in >> amt;
    }

    /**
     * Arithmetic Right Shift value `v` with carry
     * @param v value to shift
     * @param amt amount to shift
     * @param c_out output carry flag or nullptr
     * @return `v` shifted by amt
     */
    template<typename T>
    static __force_inline T arithmetic_right(T in, uint amt, bool *c_out) {
      static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

      if (c_out != nullptr) {
        if (amt) {
          *c_out = Bit::test(in, amt - 1);
        } else {
          *c_out = Bit::test(in, Bit::count<T> - 1);
        }
      }

      return sign_extend(in >> amt, Bit::count<T> - amt);
    }

    /**
     * Rotate Right value `v` with carry
     * @param v value to rotate
     * @param amt amount to shift
     * @param c_out output carry flag or nullptr
     * @return `v` shifted by amt
     */
    template<typename T>
    static __force_inline T rotate_right(T in, uint amt, bool *c_out) {
      static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value);

      // if we hit this at any point, then we'll need a safer implementation
      assert(amt < Bit::count<T>);

      if (c_out != nullptr) {
        if (amt) {
          *c_out = Bit::test(in, amt - 1);
        } else {
          *c_out = Bit::test(in, Bit::count<T> - 1);
        }
      }

      return (in << amt) | (in >> (-amt & (Bit::count<T> - 1)));
    }
  }// namespace ShiftWithCarry
}// namespace Bit
