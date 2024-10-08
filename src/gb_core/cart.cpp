#include <cassert>
#include <numeric>
#include <vector>

#include "cart.hpp"

#include "flags.hpp"
#include "util/bit.hpp"
#include "util/log.hpp"
#include "util/util.hpp"

#include "carts/rom.hpp"
#include "carts/mbc1.hpp"
#include "carts/mbc2.hpp"
#include "carts/mbc3.hpp"
#include "carts/mbc5.hpp"

/**
 * Cartridge Data
 */
std::string get_ram_file_name(const std::string& rom_file_name) {
    std::string ram_file_name = rom_file_name;
    auto ext = rom_file_name.find_last_of('.') + 1;

    ram_file_name.erase(ext, std::string::npos);
    ram_file_name.append("sav");

    return ram_file_name;
}

bool Cartridge::loadRAMFile(const std::string& ram_file_name, std::vector<u8> &ram_buffer) {
    if(!Silver::File::fileExists(ram_file_name)) {
        LogWarn("Cartridge") << ram_file_name << " does not exist";
        return false;
    }

    Silver::File *ram_file = Silver::File::openFile(ram_file_name);
    if(ram_file == nullptr) {
        LogWarn("Cartridge") << ram_file_name << " could not be opened";
        return false;
    }

    if(ram_file->getSize() != getRAMSize()) {
        LogWarn("Cartridge") << ram_file_name << "incorrect size";
        return false;
    }

    ram_file->toVector(ram_buffer);
    delete ram_file;

    LogInfo("Cartridge") << ram_file_name << " loaded";
    return true;
}

bool Cartridge::saveRAMFile(const std::string& ram_file_name, std::vector<u8> const& ram_buffer) {
    Silver::File *ram_file;
    if(Silver::File::fileExists(ram_file_name)) {
        ram_file = Silver::File::openFile(ram_file_name, true, true);
    }
    else {
        ram_file = Silver::File::createFile(ram_file_name);
    }

    if(ram_file == nullptr) {
        LogError("Cartridge") << ram_file_name << "could not be opened/created";
        return false;
    }

    LogInfo("Cartridge") << ram_file_name << " saved";
    ram_file->fromVector(ram_buffer);
    delete ram_file;

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
        LogFatal("Cartridge") << "MBC2 not supported, emulator will now crash";
        //controller = new MBC2_Controller(cart_type, rom, ram);
    } else if (cart_type.MBC3) {
        controller = new MBC3_Controller(cart_type, rom, ram);
    } else if (cart_type.MBC5) {
        //controller = new MBC5_Controller(cart_type, rom, ram);
        LogFatal("Cartridge") << "MBC5 not supported, emulator will now crash";
    } else if (cart_type.MBC6) {
        LogFatal("Cartridge") << "MBC6 not supported, emulator will now crash";
    } else if (cart_type.MBC7) {
        LogFatal("Cartridge") << "MBC7 not supported, emulator will now crash";
    } else if (cart_type.HuC1) {
        LogFatal("Cartridge") << "HuC1 not supported, emulator will now crash";
    } else if (cart_type.HuC3) {
        LogFatal("Cartridge") << "HuC3 not supported, emulator will now crash";
    }

    // TODO: handle this better?
    if (!controller) {
        assert(false);
    }

    //debug info
    LogInfo("Cartridge") << "loaded cartridge: " << getCartTitle();
    LogInfo("Cartridge") << "cart:      " << getCartTitle();
    LogInfo("Cartridge") << "cart type: " << (std::string)getCartType();
    LogInfo("Cartridge") << "cart rom:  " << getROMSize();
    if(cart_type.RAM) {
        LogInfo("Cartridge") << "cart ram:  " << getRAMSize();
        if(cart_type.BATTERY) {
            LogInfo("Cartridge") << ", Battery-Backed";
        }
        else {
            LogInfo("Cartridge");
        }
    }
}

Cartridge::~Cartridge() {
    if (cart_type.BATTERY) {
        saveRAMFile(get_ram_file_name(rom_file->getFilename()), controller->ram_data);
    }

    delete controller;
};

bool Cartridge::checkMagicNumber() const {
    u8 buf[Cartridge_Constants::MAGIC_NUM_LENGTH];
    rom_file->getBuffer(Cartridge_Constants::MAGIC_NUM_OFFSET, buf, Cartridge_Constants::MAGIC_NUM_LENGTH);
    return byteCompare(Cartridge_Constants::MAGIC_NUM, buf, Cartridge_Constants::MAGIC_NUM_LENGTH);
}

std::string Cartridge::getCartTitle() const {
    u8 buf[Cartridge_Constants::GB_TITLE_LENGTH + 1] = { 0 };
    rom_file->getBuffer(Cartridge_Constants::TITLE_OFFSET, buf, isCGBCart() ? Cartridge_Constants::CGB_TITLE_LENGTH : Cartridge_Constants::GB_TITLE_LENGTH);
    return {(char *)buf};
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

u8 Cartridge::getOldLicenseeCode() const {
    return rom_file->getByte(Cartridge_Constants::OLD_LICENSEE_CODE_OFFSET);
}

u16 Cartridge::getNewLicenseeCode() const {
    return (rom_file->getByte(Cartridge_Constants::NEW_LICENSEE_CODE_BYTE1_OFFSET) << 8) |
            rom_file->getByte(Cartridge_Constants::NEW_LICENSEE_CODE_BYTE2_OFFSET);
}

bool Cartridge::cartSupportsSGB() const {
    return rom_file->getByte(Cartridge_Constants::SGB_FLAG) == 0x03 &&
           rom_file->getByte(Cartridge_Constants::OLD_LICENSEE_CODE_OFFSET) == 0x33;
}

u8 Cartridge::cartSupportsGBCCompatMode() const {
    u8 old_licensee_code = rom_file->getByte(Cartridge_Constants::OLD_LICENSEE_CODE_OFFSET);
    if(old_licensee_code == 0x33) {
        return rom_file->getByte(Cartridge_Constants::NEW_LICENSEE_CODE_BYTE1_OFFSET) == '0' && // 0x30
               rom_file->getByte(Cartridge_Constants::NEW_LICENSEE_CODE_BYTE2_OFFSET) == '1';   // 0x31
    } else {
        return old_licensee_code == 1;
    }
}

u8 Cartridge::computeTitleChecksum() const {
    return std::accumulate(
        rom_file->begin() + 0x134,
        rom_file->begin() + 0x143,
        0_u8);
}

u8 Cartridge::computeHeaderChecksum() const {
    u16 x = 0;
    for(u16 i = 0x134; i <= 0x14C; i++) {
        x = x - rom_file->getByte(i) - 1;
    }

    return x & 0x00FF_u16;
}

u16 Cartridge::computeGlobalChecksum() const {
    //TODO: I didn't actually test this :)
    u16 i = std::accumulate<>(
        rom_file->begin(),
        rom_file->end(),
        0_u16);

    i -= rom_file->getByte(Cartridge_Constants::GLOBAL_CHECKSUM_HI_OFFSET);
    i -= rom_file->getByte(Cartridge_Constants::GLOBAL_CHECKSUM_LO_OFFSET);

    return i;
}

bool Cartridge::checkHeaderChecksum() const {
    return computeHeaderChecksum() == rom_file->getByte(Cartridge_Constants::HEADER_CHECKSUM_OFFSET);
}

bool Cartridge::checkGlobalChecksum() const {
    u16 global_checksum = (rom_file->getByte(Cartridge_Constants::GLOBAL_CHECKSUM_HI_OFFSET) << 8) |
                           rom_file->getByte(Cartridge_Constants::GLOBAL_CHECKSUM_LO_OFFSET);

    return computeHeaderChecksum() == global_checksum;
}

//forward IO calls to controller interface
u8   Cartridge::read(u16 offset)  { return controller->read(offset); }
void Cartridge::write(u16 offset, u8 data) { controller->write(offset, data); }