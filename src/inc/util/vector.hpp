#pragma once

#include <cstddef>
#include <vector>

#include "util/file.hpp"
#include "util/ints.hpp"

namespace Silver {
template<typename T, typename A = std::allocator<T>>
struct vector : public std::vector<T, A> {
    using size_type = typename std::vector<T, A>::size_type;
    using iterator  = typename std::vector<T, A>::iterator;

    // just like...copy all the various constructors for std::vector
    constexpr vector() noexcept: std::vector<T, A>() {}
    constexpr vector(const A &a) noexcept: std::vector<T, A>(a) {}
    constexpr vector(size_type c, const T &v, const A &a = A()): std::vector<T, A>(c, v, a) {}
    constexpr explicit vector(size_type c, const A &a = A()): std::vector<T, A>(c, a) {}
    template<class I>
    constexpr vector(I f, I l, const A &a = A()): std::vector<T, A>(f, l, a) {}
    constexpr vector(const std::vector<T, A> &o, const A &a = A()): std::vector<T, A>(o, a) {}
    constexpr vector(std::vector<T, A> &&o, const A &a = A()) noexcept: std::vector<T, A>(o, a) {}
    constexpr vector(std::initializer_list<T> i, const A &a = A()): std::vector<T, A>(i, a) {}

    iterator iter_at(u64 off) {
        auto iter = std::vector<T, A>::begin();
        std::advance(iter, off);
        return iter;
    }

    iterator append(std::vector<T, A> &i) {
        return std::vector<T, A>::insert(std::vector<T, A>::end(), i.begin(), i.end());
    }

    iterator append(iterator s, iterator e) { return std::vector<T, A>::insert(std::vector<T, A>::end(), s, e); }
};
} // namespace Silver
