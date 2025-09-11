#include "gba_core/io.hpp"

#include <nowide/iostream.hpp>
#include <stdexcept>

#include "gba_core/cart.hpp"

#include "util/bit.hpp"
#include "util/util.hpp"

IO_Bus::IO_Bus(Cartridge *cart, Silver::File *bios_file) :
    cart(cart) { }

IO_Bus::~IO_Bus() { }

void IO_Bus::tick() {
    // TODO
}

u32  IO_Bus::read(u32 offset, bool bypass) { return (u32)cart->read(offset); }

void IO_Bus::write(u16 offset, u8 data) { nowide::cerr << "IO Overwrite: " << as_hex(offset) << std::endl; }

u8   IO_Bus::read_reg(u8 loc) { return 0; }

void IO_Bus::write_reg(u8 loc, u8 data) { }

u8   IO_Bus::read_vram(u16 offset, bool bypass) { }

void IO_Bus::write_vram(u16 offset, u8 data) { }

u8   IO_Bus::read_oam(u16 offset, bool bypass) { }

void IO_Bus::write_oam(u16 offset, u8 data) { }

u8   IO_Bus::read_ram(u16 offset) { }

void IO_Bus::write_ram(u16 offset, u8 data) { }

u8   IO_Bus::read_hram(u16 offset) { }

void IO_Bus::write_hram(u16 offset, u8 data) { }

/**
 * Interface Routines
 */

/**
 * CPU Routines
 */
