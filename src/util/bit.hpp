#pragma once

#include <cassert>
#include <sstream>
#include <type_traits>
#include <unordered_map>

#include "types/primitives.hpp"
#include "flags.hpp"

namespace Bit {
    __force_inline bool  test(u64 reg,  u8 i) { return ((reg >> i) & 1); }
    __force_inline void   set(u8  *reg, u8 i) {       *reg |=  (1 << i); }
    __force_inline void   set(u16 *reg, u8 i) {       *reg |=  (1U << i); }
    __force_inline void   set(u32 *reg, u8 i) {       *reg |=  (1UL << i); }
    __force_inline void   set(u64 *reg, u8 i) {       *reg |=  (1ULL << i); }
    __force_inline u8     set(u8   reg, u8 i) { return reg |  (1 << i); }
    __force_inline u16    set(u16  reg, u8 i) { return reg |  (1U << i); }
    __force_inline u32    set(u32  reg, u8 i) { return reg |  (1UL << i); }
    __force_inline u64    set(u64  reg, u8 i) { return reg |  (1ULL << i); }
    __force_inline void reset(u8  *reg, u8 i) {       *reg &= ~(1 << i); }
    __force_inline void reset(u16 *reg, u8 i) {       *reg &= ~(1U << i); }
    __force_inline void reset(u32 *reg, u8 i) {       *reg &= ~(1UL << i); }
    __force_inline void reset(u64 *reg, u8 i) {       *reg &= ~(1ULL << i); }
    __force_inline u8   reset(u8   reg, u8 i) { return reg &  ~(1 << i); }
    __force_inline u16  reset(u16  reg, u8 i) { return reg &  ~(1U << i); }
    __force_inline u32  reset(u32  reg, u8 i) { return reg &  ~(1UL << i); }
    __force_inline u64  reset(u64  reg, u8 i) { return reg &  ~(1ULL << i); }

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

    template<typename T> __force_inline
    T sign_extend(T reg, u8 sign_bit) {
        return (reg ^ (1U << sign_bit)) - (1U << sign_bit);
    }

    template<typename T>
    struct shift_with_carry {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "");
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
            // if we hit this at any ppoint, then we'll need a safer implementation
            assert (rotate_amt < bit_count);
            if(carry_out != nullptr) {
                if(rotate_amt) *carry_out = Bit::test(in, rotate_amt - 1);
                else           *carry_out = Bit::test(in, bit_count - 1);
            }

            return (in<<rotate_amt) | (in>>(-rotate_amt & (bit_count - 1)));
        }
    };

    template<typename T>
    class BitFieldAccessor {
        static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "");
        static constexpr u8 bit_count = CHAR_BIT * sizeof(T);

        using key_type = std::string;

        // string: {offset, mask}
        std::unordered_map<key_type, std::pair<u8,T>> fields;

    public:
        BitFieldAccessor() {}

        BitFieldAccessor & add_field(key_type const& name, T mask) {
            return add_field(name, mask);
        }

        BitFieldAccessor const& add_field(key_type const& name, T mask) const {
            bool added = fields.insert(name, {__builtin_ctz(mask) + 1, mask}).first;
            assert(added); //TODO

            return *this;
        }

        BitFieldAccessor & add_field(key_type const& name, u8 count, u8 offset) {
            return add_field(name, count, offset);
        }

        BitFieldAccessor const& add_field(key_type const& name, u8 count, u8 offset) const {
            bool added = fields.insert(name, {((1 << count) - 1) << offset, offset});
            assert(added); //TODO

            return *this;
        }

        T get(T word, key_type mask) {
            auto val = fields.at(mask);

            return (word & val.second) >> val.first;
        }

        template<typename R>
        R get(T word, key_type mask) {
            auto val = fields.at(mask);

            return (R)((word & val.second) >> val.first);
        }

        std::string to_String() {
            std::stringstream ss;
            for(auto const& entry : fields) {
                ss << entry.first << " : " << entry.second.second << std::endl;
            }

            return ss.str();
        }

        operator std::string() {
            return this->to_String();
        }
    };
};