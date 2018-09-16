#pragma once

#include "cfg.hpp"
#include "util/ints.hpp"

class Video_Controller {
public:
    Video_Controller();
    ~Video_Controller();

    u8 read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    u8 read_ram(u8 loc);
    void write_ram(u8 loc, u8 data);

    u8 read_oam(u8 loc);
    void write_oam(u8 loc, u8 data);
};