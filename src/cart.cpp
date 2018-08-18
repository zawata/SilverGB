#include "cart.hpp"

const u8 Cartridge_Constants::MAGIC_NUM[Cartridge_Constants::MAGIC_NUM_LENGTH] = {
        0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
        0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
        0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
        0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
        0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
        0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E };

Cartridge::Cartridge(File_Interface *f) :
rom_file(f),
cart_type(Cartridge_Constants::cart_type_t::determineCartType(rom_file->getByte(Cartridge_Constants::CART_TYPE_OFFSET))) {}

Cartridge::~Cartridge() {}

u16 Cartridge::getCodeOffset() {
    return
            rom_file->getByte(Cartridge_Constants::START_CODE_HI_OFFSET) << 8 |
            rom_file->getByte(Cartridge_Constants::START_CODE_LO_OFFSET);
}

bool Cartridge::checkMagicNumber() {
    u8 buf[Cartridge_Constants::MAGIC_NUM_LENGTH];
    rom_file->getBuffer(Cartridge_Constants::MAGIC_NUM_OFFSET, buf, Cartridge_Constants::MAGIC_NUM_LENGTH);
    return Utility_Functions::byteCompare(Cartridge_Constants::MAGIC_NUM, buf, 48);
}

std::string Cartridge::getCartTitle() {
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

u8 Cartridge::getCartVersion() {
    return rom_file->getByte(Cartridge_Constants::VERSION_NUMBER);
}

Cartridge_Constants::cart_type_t Cartridge::getCartType() {
    return cart_type;
}

u32 Cartridge::getROMSize() {
    u8 rom_size_byte = rom_file->getByte(Cartridge_Constants::ROM_SIZE_OFFSET);
    switch(rom_size_byte) {
    case 0x00:
        return 32768;
    case 0x01:
        return 65536;
    case 0x02:
        return 131072;
    case 0x03:
        return 262144;
    case 0x04:
        return 524288;
    case 0x05:
        return 1048576;
    case 0x06:
        return 2097152;
    case 0x07:
        return 4194304;
    case 0x08:
        return 8388608;

    //until I can figure out what these calculate too return not supported
    case 0x52:
    case 0x53:
    case 0x54:
    default:
        return -1;
    }
}

u32 Cartridge::getRAMSize() {
    u8 rom_size_byte = rom_file->getByte(Cartridge_Constants::RAM_SIZE_OFFSET);
    switch(rom_size_byte) {
    case 0x00:
        return 0;
    case 0x01:
        return 2048;
    case 0x02:
        return 8192;
    case 0x03:
        return 32768;
    case 0x04:
        return 131072;
    case 0x05:
        return 65536;
    default:
        return -1;
    }
}

bool Cartridge::isCGBCart() {
    return rom_file->getByte(Cartridge_Constants::CGB_FLAG) & 0x80;
}

bool Cartridge::isCGBOnlyCart() {
    return rom_file->getByte(Cartridge_Constants::CGB_FLAG) == 0xC0;
}

bool Cartridge::cartSupportsSGB() {
    return rom_file->getByte(Cartridge_Constants::SGB_FLAG) == 0x03;
}


bool Cartridge::checkHeaderChecksum() {
    u16 x = 0;
    for(u16 i = 0x134; i <= 0x14C; i++){
        x = x - rom_file->getByte(i) - 1;
    }

    return (x & 0x00FF) == rom_file->getByte(Cartridge_Constants::HEADER_CHECKSUM_OFFSET);
}

bool Cartridge::checkGlobalChecksum() {
    //return false until i care enough to figure out how this is calculated.
    // real gameboy hardware doesn't even use this.
    return false;
}

u8 Cartridge::readFromCart(u16 offset) {
    if(cart_type.ROM) {
        if(offset <= 0x7FFF) {
            return rom_file->getByte(offset);
        } else if(cart_type.RAM && offset >= 0xA000 && offset <= 0xBFFF) {
            return ram_file->getByte(offset - 0xA000);
        } else {
            std::cout << "read out of bounds" << std::endl;
        }
    } else {
        // cart not supported yet
    }
}