#include <cassert>
#include <vector>

#include <nowide/iostream.hpp>

#include "gb_core/cart.hpp"

#include "util/file.hpp"
#include "util/util.hpp"
#include "util/ints.hpp"
#include "util/bit.hpp"

#include "gb_core/defs.hpp"

#include "gb_core/carts/rom.hpp"
#include "gb_core/carts/mbc1.hpp"
#include "gb_core/carts/mbc2.hpp"
#include "gb_core/carts/mbc3.hpp"
#include "gb_core/carts/mbc5.hpp"

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

bool Cartridge::loadRAMFile(std::string ram_file_name, std::vector<u8> &ram_buffer) {
    if(!Silver::File::fileExists(ram_file_name)) {
        nowide::cerr << "SAV File: does not exist" << std::endl;
        return false;
    }

    Silver::File *ram_file = Silver::File::openFile(ram_file_name);
    if(ram_file == nullptr) {
        nowide::cerr << "SAV File: could not be opened" << std::endl;
        return false;
    }

    if(ram_file->getSize() != getRAMSize()) {
        nowide::cerr << "SAV File: incorrect size" << std::endl;
        return false;
    }

    ram_file->toVector(ram_buffer);

    nowide::cout << "SAV File: loaded" << std::endl;
    return true;
}

bool Cartridge::saveRAMFile(std::string ram_file_name, std::vector<u8> const& ram_buffer) {
    Silver::File *ram_file;
    if(Silver::File::fileExists(ram_file_name)) {
        ram_file = Silver::File::openFile(ram_file_name, true, true);
    }
    else {
        ram_file = Silver::File::createFile(ram_file_name);
    }

    if(ram_file == nullptr) {
        nowide::cerr << "SAV File: could not be opened/created" << std::endl;
        return false;
    }

    nowide::cout << "SAV File: saved" << std::endl;
    ram_file->fromVector(ram_buffer);

    return true;
}

Cartridge::Cartridge(Silver::File *f) :
rom_file(f),
cart_type(Cartridge_Constants::cart_type_t::getCartType(rom_file->getByte(Cartridge_Constants::CART_TYPE_OFFSET))) {
    std::vector<u8> rom;
    std::vector<u8> ram;

    f->toVector(rom);
    assert(rom.size() == getROMSize());

    //open ram info
    if(cart_type.RAM && getRAMSize() > 0) {
        ram.resize(getRAMSize());

        if(cart_type.BATTERY) {
            loadRAMFile(get_ram_file_name(rom_file->getFilename()), ram);
        }
    }

    if(cart_type.ROM) {
        controller = new ROM_Controller(cart_type, rom, ram);
    } else if (cart_type.MBC1) {
        controller = new MBC1_Controller(cart_type, rom, ram);
    } else if (cart_type.MBC2) {
        nowide::cerr << "MBC2 not supported, emulator will now segfault" << std::endl;
        //controller = new MBC2_Controller(cart_type, rom, ram);
    } else if (cart_type.MBC3) {
        controller = new MBC3_Controller(cart_type, rom, ram);
    } else if (cart_type.MBC5) {
        //controller = new MBC5_Controller(cart_type, rom, ram);
        nowide::cerr << "MBC5 not supported, emulator will now segfault" << std::endl;
    } else if (cart_type.MBC6) {
        nowide::cerr << "MBC6 not supported, emulator will now segfault" << std::endl;
    } else if (cart_type.MBC7) {
        nowide::cerr << "MBC7 not supported, emulator will now segfault" << std::endl;
    } else if (cart_type.HuC1) {
        nowide::cerr << "HuC1 not supported, emulator will now segfault" << std::endl;
    } else if (cart_type.HuC3) {
        nowide::cerr << "HuC3 not supported, emulator will now segfault" << std::endl;
    }

    //debug info
    nowide::cout << "cart:      " << getCartTitle() << std::endl;
    nowide::cout << "cart type: " << (std::string)getCartType() << std::endl;
    nowide::cout << "cart rom:  " << getROMSize() << std::endl;
    if(cart_type.RAM) {
        nowide::cout << "cart ram:  " << getRAMSize();
        if(cart_type.BATTERY) {
            nowide::cout << ", Battery-Backed" << std::endl;
        }
        else {
            nowide::cout << std::endl;
        }
    }
}

Cartridge::~Cartridge() {
    if(cart_type.BATTERY) {
        saveRAMFile(get_ram_file_name(rom_file->getFilename()), controller->ram_data);
    }
};

bool Cartridge::checkMagicNumber() const {
    u8 buf[Cartridge_Constants::MAGIC_NUM_LENGTH];
    rom_file->getBuffer(Cartridge_Constants::MAGIC_NUM_OFFSET, buf, Cartridge_Constants::MAGIC_NUM_LENGTH);
    return byteCompare(Cartridge_Constants::MAGIC_NUM, buf, Cartridge_Constants::MAGIC_NUM_LENGTH);
}

std::string Cartridge::getCartTitle() const {
    if(isCGBCart()) {
        u8 buf[Cartridge_Constants::CGB_TITLE_LENGTH];
        rom_file->getBuffer(Cartridge_Constants::TITLE_OFFSET, buf, Cartridge_Constants::CGB_TITLE_LENGTH);
        return std::string((char *)buf);
    } else {
        u8 buf[Cartridge_Constants::GB_TITLE_LENGTH];
        rom_file->getBuffer(Cartridge_Constants::TITLE_OFFSET, buf, Cartridge_Constants::GB_TITLE_LENGTH);
        return std::string((char *)buf);
    }
}

u8 Cartridge::getCartVersion() const {
    return rom_file->getByte(Cartridge_Constants::VERSION_NUMBER);
}

Cartridge_Constants::cart_type_t Cartridge::getCartType() const {
    return cart_type;
}


Cartridge_Constants::rom_size_t Cartridge::getROMSize() const {
    using namespace Cartridge_Constants;

    u8 rom_size_byte = rom_file->getByte(Cartridge_Constants::ROM_SIZE_OFFSET);
    switch(rom_size_byte) {
    case 0x00:
        return ROM_SZ_32K;
    case 0x01:
        return ROM_SZ_64K;
    case 0x02:
        return ROM_SZ_128K;
    case 0x03:
        return ROM_SZ_256K;
    case 0x04:
        return ROM_SZ_512K;
    case 0x05:
        return ROM_SZ_1M;
    case 0x52:
        return ROM_SZ_1_1M;
    case 0x53:
        return ROM_SZ_1_2M;
    case 0x54:
        return ROM_SZ_1_5M;
    case 0x06:
        return ROM_SZ_2M;
    case 0x07:
        return ROM_SZ_4M;
    case 0x08:
        return ROM_SZ_8M;
    default:
        return ROM_SZ_INVALID;
    }
}

Cartridge_Constants::ram_size_t Cartridge::getRAMSize() const {
    using namespace Cartridge_Constants;

    u8 rom_size_byte = rom_file->getByte(Cartridge_Constants::RAM_SIZE_OFFSET);
    switch(rom_size_byte) {
    case 0x00:
        return RAM_SZ_0K;
    case 0x01:
        return RAM_SZ_2K;
    case 0x02:
        return RAM_SZ_8K;
    case 0x03:
        return RAM_SZ_32K;
    case 0x05:
        return RAM_SZ_64K;
    case 0x04:
        return RAM_SZ_128K;
    default:
        return RAM_SZ_INVALID;
    }
}

bool Cartridge::isCGBCart() const {
    return (bool)(rom_file->getByte(Cartridge_Constants::CGB_FLAG) & 0x80);
}

bool Cartridge::isCGBOnlyCart() const {
    return rom_file->getByte(Cartridge_Constants::CGB_FLAG) == 0xC0;
}

bool Cartridge::cartSupportsSGB() const {
    return rom_file->getByte(Cartridge_Constants::SGB_FLAG) == 0x03;
}


bool Cartridge::checkHeaderChecksum() const {
    u16 x = 0;
    for(u16 i = 0x134; i <= 0x14C; i++) {
        x = x - rom_file->getByte(i) - 1;
    }

    return (x & 0x00FF_u16) == rom_file->getByte(Cartridge_Constants::HEADER_CHECKSUM_OFFSET);
}

bool Cartridge::checkGlobalChecksum() const {
    //return false until I care enough to figure out how this is calculated.
    // real gameboy hardware doesn't even use this.
    return false;
}

//forward IO calls to controller interface
u8   Cartridge::read(u16 offset)  { return controller->read(offset); }
void Cartridge::write(u16 offset, u8 data) { controller->write(offset, data); }