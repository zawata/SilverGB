#pragma once
#include <string>

#include "util/file.hpp"
#include "util/ints.hpp"

class Cartridge {
public:
    Cartridge(Silver::File *f);
    ~Cartridge();

    bool loadRAMFile(std::string ram_file_name, std::vector<u32> &ram_buffer);
    bool saveRAMFile(std::string ram_file_name, std::vector<u32> const& ram_buffer);

    u32 read(u32 loc);
    void write(u32 loc, u32 data);

private:
    Silver::File *rom_file;
    std::vector<u32> rom;
    std::vector<u32> ram;
};