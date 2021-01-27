#pragma once

#include "gb_core/cart.hpp"
#include "util/bit.hpp"
#include "util/util.hpp"

#include "mbc_base.hpp"

/**
 * MBC3
 */
struct MBC3_Controller : public MBC1_Base {
    MBC3_Controller(Cartridge_Constants::cart_type_t cart_type, std::vector<u8> rom, std::vector<u8> ram) :
    MBC1_Base(cart_type, rom, ram) {}

    u8 read(u16 offset) override {
        if(bounded(offset, 0x0000_u16, 0x7FFF_u16)) {
            return MBC1_Base::read(offset);
        }
        else if(bounded(offset, 0xA000_u16, 0xBFFF_u16)) {
            if(read_ram) {
                return MBC1_Base::read(offset);
            } else {
                return latched.data[active_reg];
            }
        }
        else {
            std::cerr << "MBC3 read out of bounds: " << offset << std::endl;
            return 0;
        }
    }

    void write(u16 offset, u8 data) override {
        if(bounded(offset, 0x0_u16, 0x1FFF_u16)) {
            MBC1_Base::set_ram_enable((data & 0xf) == 0xA);
        }
        else if(bounded(offset, 0x2000_u16, 0x3FFF_u16)) {
            u8 rom_bank_num = data & 0x7F;
            if(!rom_bank_num)
                rom_bank_num++; //correct rom bank from 0
            MBC1_Base::set_rom_bank(rom_bank_num);
        }
        else if(bounded(offset, 0x4000_u16, 0x5FFF_u16)) {
            if(Bit.test(data, 3) && data <= 0xC) {
                read_ram = false;
                active_reg = data - 0x8;
            } else {
                read_ram = true;
                set_ram_bank(data & 0x3);
            }
        }
        else if(bounded(offset, 0x6000_u16, 0x7FFF_u16)) {
            static bool latch = false;

            //simulate rising edge detection;
            bool new_latch = Bit.test(data, 0);
            if(!latch && new_latch) {
                latched = active;
            }
            latch = new_latch;

        }
        else if(bounded(offset, 0xA000_u16, 0xBFFF_u16)) {
            MBC1_Base::write(offset, data);
        }
        else {
            std::cerr << "MBC3 write out of bounds: " << offset << std::endl;
        }
    }

    void tick() override {
        static u32 cntr = 0;

        if(!active.regs.halt) {
            cntr++;
        }

        //only tick once per second
        if(cntr == 4194304 ) {
            cntr = 0;

            active.regs.seconds++;
            if(active.regs.seconds == 60) {
                active.regs.seconds = 0;

                active.regs.minutes++;
                if(active.regs.minutes == 60) {
                    active.regs.minutes = 0;

                    active.regs.hours++;
                    if(active.regs.hours == 24) {
                        active.regs.hours = 0;

                        active.regs.days++;
                        if(active.regs.days == 0x1FF) {
                            active.regs.days = 0;
                            active.regs.carry = 1;
                        }
                    }
                }
            }
        }
    }

private:
    int active_reg;
    bool read_ram;

    union {
        struct rtc_regs {
            u8 seconds;
            u8 minutes;
            u8 hours;
            u16 days : 9;
            u8 __pad : 5;
            u8 halt  : 1;
            u8 carry : 1;
        } regs;
        u8 data[5];
    } active = {{0}}, latched = {{0}};
};