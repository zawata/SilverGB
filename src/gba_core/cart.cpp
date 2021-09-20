#include <cassert>
#include <vector>

#include "gba_core/cart.hpp"

#include "util/file.hpp"
#include "util/util.hpp"
#include "util/ints.hpp"
#include "util/bit.hpp"

/**
 * Cartridge Data
 */
std::string get_ram_file_name(std::string rom_file_name) {
    std::string ram_file_name = rom_file_name;
    auto ext = rom_file_name.find_last_of('.') + 1;

    ram_file_name.erase(ext, std::string::npos);
    ram_file_name.append("sav");

    return ram_file_name;
}

bool Cartridge::loadRAMFile(std::string ram_file_name, std::vector<u32> &ram_buffer) {
    if(!Silver::File::fileExists(ram_file_name)) {
        std::cout<< "SAV File: does not exist" << std::endl;
        return false;
    }

    Silver::File *ram_file = Silver::File::openFile(ram_file_name);
    if(ram_file == nullptr) {
        std::cout<< "SAV File: could not be opened" << std::endl;
        return false;
    }

    // if(ram_file->getSize() != getRAMSize()) {
    //     std::cout<< "SAV File: incorrect size" << std::endl;
    //     return false;
    // }

    ram_file->toVector<u32>(ram_buffer);

    std::cout<< "SAV File: loaded" << std::endl;
    return true;
}

bool Cartridge::saveRAMFile(std::string ram_file_name, std::vector<u32> const& ram_buffer) {
    Silver::File *ram_file;
    if(Silver::File::fileExists(ram_file_name)) {
        ram_file = Silver::File::openFile(ram_file_name, true, true);
    }
    else {
        ram_file = Silver::File::createFile(ram_file_name);
    }

    if(ram_file == nullptr) {
        std::cout<< "SAV File: could not be opened/created" << std::endl;
        return false;
    }

    std::cout<< "SAV File: saved" << std::endl;
    ram_file->fromVector(ram_buffer);

    return true;
}

Cartridge::Cartridge(Silver::File *f) :
rom_file(f) {
    f->toVector<u32>(rom);

    //TODO: Cartridge RAM
}

Cartridge::~Cartridge() {};

u32   Cartridge::read(u32 offset)  { return rom.at(offset); }
void Cartridge::write(u32 offset, u32 data) {
    std::cout << "tried to write " << as_hex(data) << " to " << as_hex(offset) << std::endl;
}