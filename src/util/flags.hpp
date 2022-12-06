#pragma once

#if defined(_MSC_VER)
    #define IS_MSVC 1
    #define IS_GCC 0
#elif defined(__GNUC__)
    #define IS_MSVC 0
    #define IS_GCC 1
#endif

#if IS_MSVC
    #define __force_inline __forceinline
#elif IS_GCC
    #define __force_inline __attribute__((always_inline)) inline

    #define CHAR_BIT __CHAR_BIT__
#endif

[[noreturn]] __force_inline static void unreachable() {
#if IS_MSVC
    __assume(false);
#elif IS_GCC
    __builtin_unreachable();
#endif
}