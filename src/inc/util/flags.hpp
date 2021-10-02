#pragma once

#if defined(_MSC_VER)
    #define __force_inline __forceinline
#elif defined(__GNUC__)
    #define __force_inline __attribute__((always_inline)) inline

    #define CHAR_BIT __CHAR_BIT__
#endif