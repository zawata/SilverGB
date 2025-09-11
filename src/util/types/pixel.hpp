#pragma once

#include <cstring>
#include <forward_list>
#include <type_traits>
#include <utility>

#include "util/flags.hpp"

#include "primitives.hpp"
#include "vector.hpp"

namespace Silver {
    enum PixelFormat { RGB, BGR, RGBA, BGRA };

    struct Pixel {
        u8           r, g, b, a = 255;

        static Pixel makeFromRGB15(u16 color) {
            u8 r = ((color >> 0) & 0x001F);
            u8 g = ((color >> 5) & 0x001F);
            u8 b = ((color >> 10) & 0x001F);
            return makeFromRGB555(r, g, b);
        }

        static Pixel makeFromRGB555(u8 r, u8 g, u8 b) {
            return {
                .r = static_cast<u8>((r << 3) | (r >> 2)),
                .g = static_cast<u8>((g << 3) | (g >> 2)),
                .b = static_cast<u8>((b << 3) | (b >> 2))};
        }
    };

    template<typename T>
    struct PixelBufferEncoder {
        template<PixelFormat p>
        struct WritePixel {
            static const int pixWidth;

            static void      write(T *dest, const Pixel &pix);
        };

        // fun fact, this is valid in c++17, but not in gcc
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
        template<>
        struct WritePixel<PixelFormat::BGR> {
            static const int pixWidth = 3;

            static void      write(T *dest, const Pixel &pixel) {
                *dest++ = pixel.b;
                *dest++ = pixel.g;
                *dest++ = pixel.r;
            }
        };

        template<>
        struct WritePixel<PixelFormat::RGB> {
            static const int pixWidth = 3;

            static void      write(T *dest, const Pixel &pixel) {
                *dest++ = pixel.r;
                *dest++ = pixel.g;
                *dest++ = pixel.b;
            }
        };

        template<>
        struct WritePixel<PixelFormat::BGRA> {
            static const int pixWidth = 4;

            static void      write(T *dest, const Pixel &pixel) {
                *dest++ = pixel.b;
                *dest++ = pixel.g;
                *dest++ = pixel.r;
                *dest++ = pixel.a;
            }
        };

        template<>
        struct WritePixel<PixelFormat::RGBA> {
            static const int pixWidth = 4;

            static void      write(T *dest, const Pixel &pixel) {
                *dest++ = pixel.r;
                *dest++ = pixel.g;
                *dest++ = pixel.b;
                *dest++ = pixel.a;
            }
        };

        template<PixelFormat pixelFormat>
        static void encodePixelBuffer(T *dest, size_t outSize, const std::vector<Pixel> &pixelBuf) {
            size_t bytesWritten = 0;
            auto   width        = WritePixel<pixelFormat>::pixWidth;
            for(const Pixel &pixel : pixelBuf) {
                if((outSize - bytesWritten) < width) {
                    return;
                }

                WritePixel<pixelFormat>::write(&dest[bytesWritten], pixel);
                bytesWritten += width;
            }
        }
    };
} // namespace Silver
