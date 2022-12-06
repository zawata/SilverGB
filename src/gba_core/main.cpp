#include "util/file.hpp"

#include "gba_core/cart.hpp"
#include "gba_core/cpu.hpp"
#include "gba_core/io.hpp"
#include <cstdio>

int main() {
    Silver::File *file = Silver::File::openFile("../../test_files/gba_tests/pokemon_ruby.gba");
    Cartridge *cart = new Cartridge(file);
    IO_Bus *io = new IO_Bus(cart);
    CPU *cpu = new CPU(io);

    while(getchar() == '\n') {
        cpu->tick();
    }
}