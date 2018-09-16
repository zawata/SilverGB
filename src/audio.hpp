#pragma once

#include "io_reg.hpp"
#include "util/ints.hpp"

#define WAVRAM_LEN 16

class Sound_Controller {
public:
    Sound_Controller();
    ~Sound_Controller();

    void tick();

    u8 read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    u8 read_wavram(u8 loc);
    void write_wavram(u8 loc, u8 data);
};
