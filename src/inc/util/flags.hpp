
#if defined(_MSC_VER)
    #define __force_inline __forceinline
#elif defined(__GNUC__)
    #define __attribute__((always_inline))
#endif