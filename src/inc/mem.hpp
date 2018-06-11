#include "cart.hpp"
#include "util.hpp"

#ifndef MEM_HPP
#define MEM_HPP

class Memory_Map {
public:
    Memory_Map(Cartridge *c);
    ~Memory_Map();

    u8   read(u16 offset);
    void write(u16 offset, u8 data);

private:
    Cartridge *cart;
};

#endif