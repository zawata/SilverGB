#pragma once

#include "file.hpp"

#include "util/ints.hpp"
#include <string>

struct MemoryBankController;

class Cartridge_Constants {
public:
    typedef struct __cart_type_t {
        static struct __cart_type_t getCartType(u8 cart_type) {
            switch(cart_type) {
            case 0x00: //00h  ROM ONLY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type});
            case 0x01: //01h  MBC1
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type});
            case 0x02: //02h  MBC1+RAM
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,cart_type});
            case 0x03: //03h  MBC1+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,cart_type});
            case 0x05: //05h  MBC2
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type});
            case 0x06: //06h  MBC2+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,cart_type});
            case 0x08: //08h  ROM+RAM
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,cart_type});
            case 0x09: //09h  ROM+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,cart_type});
            case 0x0B: //0Bh  MMM01
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,cart_type});
            case 0x0C: //0Ch  MMM01+RAM
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,cart_type});
            case 0x0D: //0Dh  MMM01+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,cart_type});
            case 0x0F: //0Fh  MBC3+TIMER+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,cart_type});
            case 0x10: //10h  MBC3+TIMER+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,cart_type});
            case 0x11: //11h  MBC3
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,cart_type});
            case 0x12: //12h  MBC3+RAM
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,cart_type});
            case 0x13: //13h  MBC3+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,cart_type});
            case 0x19: //19h  MBC5
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,cart_type});
            case 0x1A: //1Ah  MBC5+RAM
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,cart_type});
            case 0x1B: //1Bh  MBC5+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,cart_type});
            case 0x1C: //1Ch  MBC5+RUMBLE
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,cart_type});
            case 0x1D: //1Dh  MBC5+RUMBLE+RAM
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,cart_type});
            case 0x1E: //1Eh  MBC5+RUMBLE+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,cart_type});
            case 0x20: //20h  MBC6
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,cart_type});
            case 0x22: //22h  MBC7+SENSOR+RUMBLE+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,0,cart_type});
            case 0xFC: //FCh  POCKET CAMERA
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,cart_type});
            case 0xFD: //FDh  BANDAI TAMA5
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,cart_type});
            case 0xFE: //FEh  HuC3
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,cart_type});
            case 0xFF: //FFh  HuC1+RAM+BATTERY
                return Cartridge_Constants::cart_type_t(
                    (cart_type_t){0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,cart_type});
            default:
                return Cartridge_Constants::cart_type_t();
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

    static const u16 START_CODE_HI_OFFSET           = 0x0103;
    static const u16 START_CODE_LO_OFFSET           = 0x0102;

    static const u16 MAGIC_NUM_OFFSET               = 0x0104;
    static const u16 MAGIC_NUM_LENGTH               = 0x0030;

    static const u16 TITLE_OFFSET                   = 0x0134;
    static const u16 GB_TITLE_LENGTH                = 0x0010;
    static const u16 SGB_TITLE_LENGTH               = 0x0010;
    static const u16 CGB_TITLE_LENGTH               = 0x000F;

    static const u16 CGB_FLAG                       = 0x0143;

    static const u16 SGB_LICENSEE_CODE_BYTE1_OFFSET = 0x014B;
    static const u16 SGB_LICENSEE_CODE_BYTE2_OFFSET = 0x014B; //TODO: FIXME

    static const u16 SGB_FLAG                       = 0x0146;

    static const u16 CART_TYPE_OFFSET               = 0x0147;
    static const u16 ROM_SIZE_OFFSET                = 0x0148;
    static const u16 RAM_SIZE_OFFSET                = 0x0149;

    static const u16 DESTINATION_OFFSET             = 0x014A;

    static const u16 GB_LICENSEE_CODE_OFFSET        = 0x014B;

    static const u16 VERSION_NUMBER                 = 0x014C;

    static const u16 HEADER_CHECKSUM_OFFSET         = 0x014D;
    static const u16 GLOBAL_CHECKSUM_HI_OFFSET      = 0x014A;
    static const u16 GLOBAL_CHECKSUM_LO_OFFSET      = 0x014B;


    static const u8 MAGIC_NUM[MAGIC_NUM_LENGTH];
};

class Cartridge {
public:
    Cartridge(File_Interface *f);
    ~Cartridge();

    u16 getCodeOffset();
    bool checkMagicNumber();

    std::string getCartTitle();
    u8 getCartVersion();
    Cartridge_Constants::cart_type_t getCartType();

    u32 getROMSize();
    u32 getRAMSize();

    bool isCGBCart();
    bool isCGBOnlyCart();

    bool cartSupportsSGB();

    bool checkHeaderChecksum();
    bool checkGlobalChecksum();

    u8 read_rom(u16 loc);
    void write_rom(u16 loc, u8 data);

    u8 read_ram(u16 loc);
    void write_ram(u16 loc, u8 data);

private:
    File_Interface *rom_file;
    File_Interface *ram_file;

    MemoryBankController *controller;

    Cartridge_Constants::cart_type_t cart_type;
};