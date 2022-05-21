#pragma once

#include "util/file.hpp"
#include "util/ints.hpp"
#include "util/vector.hpp"

#include <string>

#if !defined(_MSC_VER)
#define return_cart_type(...) { \
    return (__cart_type_t){__VA_ARGS__}; \
}
#else
#define return_cart_type(...) { \
    __cart_type_t o = {__VA_ARGS__}; \
    return o; \
}
#endif

namespace Cartridge_Constants {
    typedef struct __cart_type_t {
        static struct __cart_type_t getCartType(u8 cart_type) {
            switch(cart_type) {
            //TODO: this could be a bunch of binary definitions instead of ...this
            case 0x00: return_cart_type(1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type) //00h  ROM ONLY
            case 0x01: return_cart_type(0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type) //01h  MBC1
            case 0x02: return_cart_type(0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,cart_type) //02h  MBC1+RAM
            case 0x03: return_cart_type(0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,cart_type) //03h  MBC1+RAM+BATTERY
            case 0x05: return_cart_type(0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type) //05h  MBC2
            case 0x06: return_cart_type(0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,cart_type) //06h  MBC2+BATTERY
            case 0x08: return_cart_type(1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,cart_type) //08h  ROM+RAM
            case 0x09: return_cart_type(1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,cart_type) //09h  ROM+RAM+BATTERY
            case 0x0B: return_cart_type(0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type) //0Bh  MMM01
            case 0x0C: return_cart_type(0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,cart_type) //0Ch  MMM01+RAM
            case 0x0D: return_cart_type(0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,cart_type) //0Dh  MMM01+RAM+BATTERY
            case 0x0F: return_cart_type(0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,cart_type) //0Fh  MBC3+TIMER+BATTERY
            case 0x10: return_cart_type(0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,cart_type) //10h  MBC3+TIMER+RAM+BATTERY
            case 0x11: return_cart_type(0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,cart_type) //11h  MBC3
            case 0x12: return_cart_type(0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,cart_type) //12h  MBC3+RAM
            case 0x13: return_cart_type(0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,cart_type) //13h  MBC3+RAM+BATTERY
            case 0x19: return_cart_type(0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,cart_type) //19h  MBC5
            case 0x1A: return_cart_type(0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,cart_type) //1Ah  MBC5+RAM
            case 0x1B: return_cart_type(0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,cart_type) //1Bh  MBC5+RAM+BATTERY
            case 0x1C: return_cart_type(0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,cart_type) //1Ch  MBC5+RUMBLE
            case 0x1D: return_cart_type(0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,cart_type) //1Dh  MBC5+RUMBLE+RAM
            case 0x1E: return_cart_type(0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,cart_type) //1Eh  MBC5+RUMBLE+RAM+BATTERY
            case 0x20: return_cart_type(0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,cart_type) //20h  MBC6
            case 0x22: return_cart_type(0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,0,cart_type) //22h  MBC7+SENSOR+RUMBLE+RAM+BATTERY
            case 0xFC: return_cart_type(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,cart_type) //FCh  POCKET CAMERA
            case 0xFD: return_cart_type(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,cart_type) //FDh  BANDAI TAMA5
            case 0xFE: return_cart_type(0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,cart_type) //FEh  HuC3
            case 0xFF: return_cart_type(0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,cart_type) //FFh  HuC1+RAM+BATTERY
            default:
                Cartridge_Constants::__cart_type_t c = { 0 };
                return c;
            }
        }

        operator std::string() const {
            std::string ret = std::to_string(code) + ": ";
            if (ROM)
                ret += "ROM+";
            if (MMMO1)
                ret += "MMMO1+";
            if (MBC1)
                ret += "MBC1+";
            if (MBC2)
                ret += "MBC2+";
            if (MBC3)
                ret += "MBC3+";
            if (MBC5)
                ret += "MBC5+";
            if (MBC6)
                ret += "MBC6+";
            if (MBC7)
                ret += "MBC7+";
            if (TIMER)
                ret += "TIMER+";
            if (HuC1)
                ret += "HuC1+";
            if (HuC3)
                ret += "HuC3+";
            if (SENSOR)
                ret += "SENSOR+";
            if (RUMBLE)
                ret += "RUMBLE+";
            if (RAM)
                ret += "RAM+";
            if (BATTERY)
                ret += "BATTERY+";
            if (POCKET_CAMERA)
                ret += "POCKET_CAMERA+";
            if (BANDAI_TAMA5)
                ret += "BANDAI_TAMA5+";
            ret.pop_back(); //remove the last plus
            return ret;
        }
        bool ROM;
        bool MMMO1;
        bool MBC1;
        bool MBC2;
        bool MBC3;
        bool MBC5;
        bool MBC6;
        bool MBC7;
        bool TIMER;
        bool HuC1;
        bool HuC3;
        bool SENSOR;
        bool RUMBLE;
        bool RAM;
        bool BATTERY;
        bool POCKET_CAMERA;
        bool BANDAI_TAMA5;
        int code;
    } cart_type_t;

    enum rom_size_t {
        ROM_SZ_INVALID = -1,
        ROM_SZ_32K     = 32768,
        ROM_SZ_64K     = 65536,
        ROM_SZ_128K    = 131072,
        ROM_SZ_256K    = 262144,
        ROM_SZ_512K    = 524288,
        ROM_SZ_1M      = 1048576,
        ROM_SZ_1_1M    = 1179648,
        ROM_SZ_1_2M    = 1310720,
        ROM_SZ_1_5M    = 1572864,
        ROM_SZ_2M      = 2097152,
        ROM_SZ_4M      = 4194304,
        ROM_SZ_8M      = 8388608
    };

    enum ram_size_t {
        RAM_SZ_INVALID = -1,
        RAM_SZ_0K      = 0,
        RAM_SZ_2K      = 2048,
        RAM_SZ_8K      = 8192,
        RAM_SZ_32K     = 32768,
        RAM_SZ_64K     = 65535,
        RAM_SZ_128K    = 131072,
    };

    static const u16 START_CODE_HI_OFFSET           = 0x0103;
    static const u16 START_CODE_LO_OFFSET           = 0x0102;

    static const u16 MAGIC_NUM_OFFSET               = 0x0104;
    static const u16 MAGIC_NUM_LENGTH               = 0x0030;

    static const u16 TITLE_OFFSET                   = 0x0134;
    static const u16 GB_TITLE_LENGTH                = 0x0010;
    static const u16 SGB_TITLE_LENGTH               = 0x0010;
    static const u16 CGB_TITLE_LENGTH               = 0x000F;

    static const u16 CGB_FLAG                       = 0x0143;

    static const u16 NEW_LICENSEE_CODE_BYTE1_OFFSET = 0x0144;
    static const u16 NEW_LICENSEE_CODE_BYTE2_OFFSET = 0x0145;

    static const u16 SGB_FLAG                       = 0x0146;

    static const u16 CART_TYPE_OFFSET               = 0x0147;
    static const u16 ROM_SIZE_OFFSET                = 0x0148;
    static const u16 RAM_SIZE_OFFSET                = 0x0149;

    static const u16 DESTINATION_OFFSET             = 0x014A;

    static const u16 OLD_LICENSEE_CODE_OFFSET       = 0x014B;

    static const u16 VERSION_NUMBER                 = 0x014C;

    static const u16 HEADER_CHECKSUM_OFFSET         = 0x014D;
    static const u16 GLOBAL_CHECKSUM_HI_OFFSET      = 0x014A;
    static const u16 GLOBAL_CHECKSUM_LO_OFFSET      = 0x014B;


    static const u8 MAGIC_NUM[MAGIC_NUM_LENGTH] = {
            0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
            0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
            0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
            0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
            0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
            0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E };
}

/**
 * MBC Interface
 */
struct MemoryBankController {
    friend class Cartridge;

    virtual u8 read(u16 offset) = 0;
    virtual void write(u16 offset, u8 data) = 0;
    virtual void tick() {};
protected:
    MemoryBankController(Cartridge_Constants::cart_type_t const& cart_type, Silver::vector<u8> const& rom_data, Silver::vector<u8> const& ram_data) :
    cart_type(cart_type),
    rom_data(rom_data),
    ram_data(ram_data) {}

    Cartridge_Constants::cart_type_t cart_type;

    Silver::vector<u8> rom_data;
    Silver::vector<u8> ram_data;
};

class Cartridge {
public:
    explicit Cartridge(Silver::File *f);
    ~Cartridge();

    bool loadRAMFile(std::string ram_file_name, std::vector<u8> &ram_buffer);
    bool saveRAMFile(std::string ram_file_name, std::vector<u8> const& ram_buffer);

    bool checkMagicNumber() const;

    std::string getCartTitle() const;
    u8 getCartVersion() const;
    Cartridge_Constants::cart_type_t getCartType() const;

    Cartridge_Constants::rom_size_t getROMSize() const;
    Cartridge_Constants::ram_size_t getRAMSize() const;

    bool isCGBCart() const;
    bool isCGBOnlyCart() const;

    u8 getOldLicenseeCode() const;
    u16 getNewLicenseeCode() const;

    bool cartSupportsSGB() const;
    u8 cartSupportsGBCCompatMode() const;

    u8 computeTitleChecksum() const;
    u8 computeHeaderChecksum() const;
    u16 computeGlobalChecksum() const;

    bool checkHeaderChecksum() const;
    bool checkGlobalChecksum() const;

    u8 read(u16 loc);
    void write(u16 loc, u8 data);

private:
    Silver::File *rom_file;

    MemoryBankController *controller;

    Cartridge_Constants::cart_type_t cart_type;
};