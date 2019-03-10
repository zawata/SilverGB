#pragma once

#define as_hex(x) std::hex << (int)(x) << std::dec

#define IMAGE_FORMAT_SIZE 3 //rgb

#define GB_S_W    160                          // screen width
#define GB_S_H    144                          // screen height
#define GB_S_P    (GB_S_W * GB_S_H)            // screen pixel count
#define GB_S_P_SZ (GB_S_P * IMAGE_FORMAT_SIZE) // screen pixel buffer size