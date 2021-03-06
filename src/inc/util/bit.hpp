#pragma once

#include <type_traits>

#include "ints.hpp"

namespace Bit {
    inline bool  test(u64 reg,  u8 i) { return ((reg >> i) & 1); }

    inline void   set(u8  *reg, u8 i) { *reg |=  (1 << i); }
    inline void   set(u16 *reg, u8 i) { *reg |=  (1U << i); }
    inline void   set(u32 *reg, u8 i) { *reg |=  (1UL << i); }
    inline void   set(u64 *reg, u8 i) { *reg |=  (1ULL << i); }
    inline void reset(u8  *reg, u8 i) { *reg &= ~(1 << i); }
    inline void reset(u16 *reg, u8 i) { *reg &= ~(1U << i); }
    inline void reset(u32 *reg, u8 i) { *reg &= ~(1UL << i); }
    inline void reset(u64 *reg, u8 i) { *reg &= ~(1ULL << i); }

    inline bool fallen(u64 s_old, u64 s_new, u8 bit) {
        return test(s_old, bit) && !test(s_new, bit);
    }

    inline bool risen(u64 s_old, u64 s_new, u8 bit) {
        return !test(s_old, bit) && test(s_new, bit);
    }

    inline bool changed(u64 s_old, u64 s_new, u8 bit) {
        return test(s_old, bit) != test(s_new, bit);
    }

    template<typename T>
    class BitWatcher {
        T *item;
        T item_old;
        u8 bit;
    public:
        explicit BitWatcher(T *item, u8 bit):
        item(item),
        item_old(*item),
        bit(bit) {
            static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
        }

        bool risen() {
            bool ret = Bit::risen(item_old, *item, bit);
            item_old = *item;
            return ret;
        }

        bool fallen() {
            bool ret = Bit::fallen(item_old, *item, bit);
            item_old = *item;
            return ret;
        }

        bool changed() {
            bool ret = Bit::fallen(item_old, *item, bit);
            item_old = *item;
            return ret;
        }
    };
};

