#include "cart.hpp"
#include "util.hpp"

#ifndef IO_HPP
#define IO_HPP

class IO_Bus {
public:
    IO_Bus(Cartridge *c);
    ~IO_Bus();

    u8   read(u16 offset);
    void write(u16 offset, u8 data);

private:
    Cartridge *cart;
};

#endif