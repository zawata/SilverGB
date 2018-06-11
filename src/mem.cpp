#include "mem.hpp"

Memory_Map::Memory_Map(Cartridge *c) :
cart(c) {}

u8 Memory_Map::read(u16 offset) {
    if(offset <= 0x3FFF) {        // 16KB ROM bank 00
        cart->readFromCart(offset);
    } else if(offset <= 0x7FFF) { // 16KB ROM Bank 01~NN
        cart->readFromCart(offset);
    } else if(offset <= 0x9FFF) { // 8KB Video RAM (VRAM)

    } else if(offset <= 0xBFFF) { // 8KB External RAM

    } else if(offset <= 0xCFFF) { // 4KB Work RAM (WRAM) bank 0

    } else if(offset <= 0xDFFF) { // 4KB Work RAM (WRAM) bank 1~N

    } else if(offset <= 0xFDFF) { // Mirror of C000~DDFF (ECHO RAM)

    } else if(offset <= 0xFE9F) { // Sprite attribute table (OAM)

    } else if(offset <= 0xFEFF) { // Not Usable

    } else if(offset <= 0xFF7F) { // I/O Registers

    } else if(offset <= 0xFFFE) { // High RAM (HRAM)

    } else if(offset <= 0xFFFF) { // Interrupts Enable Register (IE)

    }
}

void Memory_Map::write(u16 offset, u8 data) {
    if(offset <= 0x3FFF) {        // 16KB ROM bank 00

    } else if(offset <= 0x7FFF) { // 16KB ROM Bank 01~NN

    } else if(offset <= 0x9FFF) { // 8KB Video RAM (VRAM)

    } else if(offset <= 0xBFFF) { // 8KB External RAM

    } else if(offset <= 0xCFFF) { // 4KB Work RAM (WRAM) bank 0

    } else if(offset <= 0xDFFF) { // 4KB Work RAM (WRAM) bank 1~N

    } else if(offset <= 0xFDFF) { // Mirror of C000~DDFF (ECHO RAM)

    } else if(offset <= 0xFE9F) { // Sprite attribute table (OAM)

    } else if(offset <= 0xFEFF) { // Not Usable

    } else if(offset <= 0xFF7F) { // I/O Registers

    } else if(offset <= 0xFFFE) { // High RAM (HRAM)

    } else if(offset <= 0xFFFF) { // Interrupts Enable Register (IE)

    }
}