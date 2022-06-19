#pragma once

// macro            compiler     platform
// MSC_VER          msvc         windows
// __GNUC__         clang/gcc    linux/macos
// __EMSCRIPTEN__   emscripten   wasm

#if defined(_MSC_VER)
    #define __force_inline __forceinline
#elif defined(__GNUC__)
    #define __force_inline __attribute__((always_inline)) inline

    #define CHAR_BIT __CHAR_BIT__
#endif

[[noreturn]] __force_inline static void unreachable() {
#if defined(_MSC_VER)
    __assume(false);
#elif defined(__GNUC__)
    __builtin_unreachable();
#endif
}