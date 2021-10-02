#include "util/file.hpp"

#include "gba_core/cart.hpp"
#include "gba_core/cpu.hpp"
#include "gba_core/io.hpp"

int main() {
    Silver::File *file = Silver::File::openFile(R"u8(D:/Pokemon - Ruby Version (USA).gba)u8");
    Cartridge *cart = new Cartridge(file);
    IO_Bus *io = new IO_Bus(cart);
    CPU *cpu = new CPU(io);
}