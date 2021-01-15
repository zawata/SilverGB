#include "gb_core/cart.hpp"

#include "util/file.hpp"
#include "util/util.hpp"
#include "util/ints.hpp"
#include "util/bit.hpp"

#include "gb_core/defs.hpp"

#include <vector>

/**
 * MBC Interface
 */
struct MemoryBankController {
    virtual u8 read_rom(u16 offset) = 0;
    virtual void write_rom(u16 offset, u8 data) = 0;
    virtual u8 read_ram(u16 offset) = 0;
    virtual void write_ram(u16 offset, u8 data) = 0;
protected:
    MemoryBankController(Silver::File *rom_file, Cartridge_Constants::cart_type_t cart_type) :
    rom_file(rom_file),
    cart_type(cart_type) {}

    Silver::File *rom_file;
    Cartridge_Constants::cart_type_t cart_type;
};

/**
 * ROM with optional RAM
 */
struct ROM_Controller : public MemoryBankController {
    ROM_Controller(Silver::File *rom_file, Cartridge_Constants::cart_type_t cart_type, std::vector<u8> ram) :
    MemoryBankController(rom_file, cart_type) {
        if(cart_type.RAM)
            this->ram = ram;
    }

    u8 read_rom(u16 offset) override {
        if(offset <= 0x7FFF) {
            return rom_file->getByte(offset);
        } else {
            std::cerr << "read out of bounds" << std::endl;
            return 0;
        }
    }

    void write_rom(u16 offset, u8 data) override {
        //We don't write to the ROM in the ROM Controller
        std::cerr << "write out of bounds" << std::endl;
    }

    u8 read_ram(u16 offset) override {
        if(offset >= 0xA000 && offset <= 0xBFFF) {
            offset -= 0xA000;
            if(cart_type.RAM)
                return ram[offset];
            else
                return 0;
        } else {
            std::cerr << "read out of bounds" << std::endl;
            return 0;
        }
    }

    void write_ram(u16 offset, u8 data) override {
        if(offset >= 0xA000 && offset <= 0xBFFF) {
            offset -= 0xA000;
            if(cart_type.RAM)
                ram[offset] = data;
        } else {
            std::cerr << "write out of bounds" << std::endl;
        }
    }

private:
    std::vector<u8> ram;
};

/**
 * MBC1
 *
 * TODO: do MBC's always have ram?
 */
struct MBC1_Controller : public MemoryBankController {
    MBC1_Controller(Silver::File *rom_file, Cartridge_Constants::cart_type_t cart_type, std::vector<u8> ram) :
    MemoryBankController(rom_file, cart_type) {
        if(cart_type.RAM)
            this->ram = ram;

        ram_enable = false; //TODO?
        bank_num = 1;
    }

    u8 read_rom(u16 offset) override {
        if(offset <= 0x3FFF) {
            return rom_file->getByte(offset);
        } else if(offset <= 0x7FFF) {
            offset -= 0x4000;

            if(!mode_select) offset += (((bank_num << 5) | rom_bank_num) * ROM_BANK_SIZE);
            else             offset += (                   rom_bank_num * ROM_BANK_SIZE);

            return rom_file->getByte(offset);
        } else {
            std::cerr << "read out of bounds" << std::endl;
            return 0;
        }
    }

    void write_rom(u16 offset, u8 data) override {
        if(offset <= 0x1FFF) {
            ram_enable = (data & 0xf) == 0xA;
        } else if(offset <= 0x3FFF) {
            rom_bank_num = data & 0x1F;
            if(!rom_bank_num) rom_bank_num++; //correct rom bank from 0
        } else if(offset <= 0x5FFF) {
            bank_num = data & 0x03;
        } else if(offset <= 0x7FFF) {
            mode_select = data & 1;
        } else {
            std::cerr << "write out of bounds" << std::endl;
        }
    }

    u8 read_ram(u16 offset) override {
        if(offset >= 0xA000 && offset <= 0xBFFF) {
            offset -= 0xA000;
            if(cart_type.RAM && ram_enable) {
                if(mode_select) return ram[offset+(bank_num * RAM_BANK_SIZE)];
                else            return ram[offset];
            }
            else {
                return 0;
            }
        } else {
            std::cerr << "read out of bounds" << std::endl;
            return 0;
        }
    }

    void write_ram(u16 offset, u8 data) override {
        if(offset >= 0xA000 && offset <= 0xBFFF) {
            if(cart_type.RAM && ram_enable) {
                offset -= 0xA000;
                if(mode_select) ram[offset+(bank_num * RAM_BANK_SIZE)] = data;
                else            ram[offset] = data;
            }
            else {
                return;
            }
        } else {
            std::cerr << "write out of bounds" << std::endl;
            return;
        }
    }

private:
    std::vector<u8> ram;

    bool ram_enable;
    u8 rom_bank_num = 0;
    u8 bank_num = 0;
    //true = ram_banking, false = rom_banking
    bool mode_select = false;

    static const u16 ROM_BANK_SIZE = 0x4000;
    static const u16 RAM_BANK_SIZE = 0x2000;
};

/**
 * MBC2
 *
 * TODO: this
 */
struct MBC2_Controller : public MemoryBankController {
    MBC2_Controller(Silver::File *rom_file, Cartridge_Constants::cart_type_t cart_type, std::vector<u8> ram) :
    MemoryBankController(rom_file, cart_type) {
        std::cerr << "MBC2 not yet implemented. Will probably crash now" << std::endl;
        if(cart_type.RAM)
            this->ram = ram;

        ram_enable = false;
    }

    u8 read_rom(u16 offset) override {
        if(offset <= 0x3FFF) {
            return rom_file->getByte(offset);
        } else if(offset <= 0x7FFF) {
            offset -= 0x4000;
            return rom_file->getByte(offset + (rom_bank_num * ROM_BANK_SIZE));
        } else {
            std::cerr << "read out of bounds" << std::endl;
            return 0;
        }
    }

    void write_rom(u16 offset, u8 data) override {
        if(offset <= 0x1FFF) {
            if(Bit.test(offset, 8)) return;

            ram_enable = (data & 0xF) == 0xA;
        } else if(offset <= 0x7FFF) {
            if(Bit.test(offset, 8)) return;

            rom_bank_num = data & 0xF;
        } else {
            std::cerr << "read out of bounds" << std::endl;
        }
    }

    u8 read_ram(u16 offset) override {
        return 0;
    }

    void write_ram(u16 offset, u8 data) override {

    }

private:
    std::vector<u8> ram;

    bool ram_enable;
    u8 rom_bank_num = 0;
    //true = ram_banking, false = rom_banking
    bool mode_select = false;

    static const u16 ROM_BANK_SIZE = 0x4000;
    static const u16 RAM_BANK_SIZE = 0x2000;
};

/**
 * MBC3
 *
 * TODO: this
 */
struct MBC3_Controller : public MemoryBankController {
    MBC3_Controller(Silver::File *rom_file, Cartridge_Constants::cart_type_t cart_type, std::vector<u8> ram) :
    MemoryBankController(rom_file, cart_type) {
        std::cerr << "MBC3 not yet implemented. Will probably crash now" << std::endl;
        if(cart_type.RAM)
            this->ram = ram;
    }

    u8 read_rom(u16 offset) override {
        return 0;
    }

    void write_rom(u16 offset, u8 data) override {}

    u8 read_ram(u16 offset) override {
        return 0;
    }

    void write_ram(u16 offset, u8 data) override {}

private:
    std::vector<u8> ram;
};

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

const u8 Cartridge_Constants::MAGIC_NUM[Cartridge_Constants::MAGIC_NUM_LENGTH] = {
        0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
        0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
        0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
        0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
        0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
        0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E };

Cartridge::Cartridge(Silver::File *f) :
rom_file(f),
cart_type(Cartridge_Constants::cart_type_t::getCartType(rom_file->getByte(Cartridge_Constants::CART_TYPE_OFFSET))) {
    std::vector<u8> ram;

    //open ram info
    if(cart_type.RAM && getRAMSize() > 0) {
        ram.reserve(getRAMSize());

        if(cart_type.BATTERY) {
            //get ram file name
            std::string ram_file_name = get_ram_file_name(rom_file->getFilename());

            //open ram file
            Silver::File *ram_file;
            if((ram_file = Silver::File::createFile(ram_file_name)) == nullptr) {
                ram_file = Silver::File::openFile(ram_file_name);
            }

            //check that fiel could be opened
            if(!ram_file) {
                std::cerr << "Error Opening " << ram_file_name << std::endl;
                std::cerr << "Ram Data will not be saved." << std::endl;
                goto ContinueLoad; //TODO: no
            }

            //check that file size matches cart ram size
            auto ram_size = ram_file->getSize();
            if(ram_size != getRAMSize()) {
                std::cerr << "Ram File " << ram_file_name << "does not match cart type" << std::endl
                          << "Data will not be loaded" << std::endl;
                goto ContinueLoad; //TODO: no
            }

            //load ram data from file
            ram_file->getBuffer(0, ram.data(), getRAMSize());
        }
    }

//TODO: I haaaaate gotos. clean this up when i care more
ContinueLoad:
    if(cart_type.ROM) {
        controller = new ROM_Controller(f, cart_type, ram);
    } else if (cart_type.MBC1) {
        controller = new MBC1_Controller(f, cart_type, ram);
    } else if (cart_type.MBC2) {
        controller = new MBC2_Controller(f, cart_type, ram);
    } else if (cart_type.MBC3) {
        controller = new MBC3_Controller(f, cart_type, ram);
    }

    //debug info
    std::cout << "cart:      " << getCartTitle() << std::endl;
    std::cout << "cart type: " << (std::string)getCartType() << std::endl;
    std::cout << "cart rom:  " << getROMSize() << std::endl;
    if(cart_type.RAM) {
        std::cout << "cart ram:  " << getRAMSize();
        if(cart_type.BATTERY) {
            std::cout << ", Battery-Backed" << std::endl;
        }
        else {
            std::cout << std::endl;
        }
    }
}

Cartridge::~Cartridge() = default;

u16 Cartridge::getCodeOffset() {
    return
            rom_file->getByte(Cartridge_Constants::START_CODE_HI_OFFSET) << 8 |
            rom_file->getByte(Cartridge_Constants::START_CODE_LO_OFFSET);
}

bool Cartridge::checkMagicNumber() {
    u8 buf[Cartridge_Constants::MAGIC_NUM_LENGTH];
    rom_file->getBuffer(Cartridge_Constants::MAGIC_NUM_OFFSET, buf, Cartridge_Constants::MAGIC_NUM_LENGTH);
    return byteCompare(Cartridge_Constants::MAGIC_NUM, buf, Cartridge_Constants::MAGIC_NUM_LENGTH);
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
        return 32768; // 2^15
    case 0x01:
        return 65536; // 2^16
    case 0x02:
        return 131072; // 2^17
    case 0x03:
        return 262144; // 2^18
    case 0x04:
        return 524288; //2^19
    case 0x05:
        return 1048576; //2^20
    case 0x06:
        return 2097152; //2^21
    case 0x07:
        return 4194304; //2^22
    case 0x08:
        return 8388608; //2^23

    //until I can figure out what these calculate to, return not supported
    case 0x52:
    case 0x53:
    case 0x54:
    default:
        return (u32)-1;
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
        return (u32)-1;
    }
}

bool Cartridge::isCGBCart() {
    return (bool)(rom_file->getByte(Cartridge_Constants::CGB_FLAG) & 0x80);
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
    //return false until I care enough to figure out how this is calculated.
    // real gameboy hardware doesn't even use this.
    return false;
}

//forward IO calls to controller interface
u8   Cartridge::read_rom(u16 offset)  { return controller->read_rom(offset); }
void Cartridge::write_rom(u16 offset, u8 data) {  controller->write_rom(offset, data); }

u8   Cartridge::read_ram(u16 offset) { return controller->read_rom(offset); }
void Cartridge::write_ram(u16 offset, u8 data) { controller->write_ram(offset, data); }