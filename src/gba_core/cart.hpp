#pragma once
#include <string>

#include "util/file.hpp"

class Cartridge {
public:
    explicit Cartridge(Silver::File *f);

    ~Cartridge();

    bool loadRAMFile(const std::string &ram_file_name, std::vector<u32> &ram_buffer);

    bool saveRAMFile(std::string ram_file_name, std::vector<u32> const &ram_buffer);

    u32  read(u32 loc) const;

    void write(u32 loc, u32 data);

private:
    Silver::File    *rom_file;
    std::vector<u32> rom;
    std::vector<u32> ram;
};
