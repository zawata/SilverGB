#include <cassert>
#include <cstring>
#include <iostream>

#include "gb_core/defs.hpp"
#include "gb_core/io_reg.hpp"
#include "gb_core/apu.hpp"

#include "util/bit.hpp"
#include "util/util.hpp"

//#define AMPLITUDE   25000.0f
#define SAMPLE_RATE 44100.0f //TODO

// inline s16 mix(s16 sample1, s16 sample2) {
//     long result((long)sample1 + (long)sample2);
//     if (0x7FFF < result)
//         return (short)0x7FFF;
//     else if ((short)0x8000 > result)
//         return (short)0x8000;
//     else
//         return result;
// }

inline u8 normalize(bool state, u8 volume) {
    if(state) return volume << 4;
    else      return 0;
}

inline bool _duty_check(u8 duty, u8 wv_count) {
//to simplify the duty cycle checking, we can break this out

// Bits  Cyc%   0 1 2 3 4 5 6 7
// 00  : 12.5%  ________==______
// 01  : 25%    ________====____
// 10  : 50%    ____========____
// 11  : 75%    ========____====
    switch(duty) {
    case 0:
        return wv_count == 4;
    case 1:
        return wv_count == 4 ||
               wv_count == 5;
    case 2:
        return wv_count == 2 ||
               wv_count == 3 ||
               wv_count == 4 ||
               wv_count == 5;
    case 3:
        return wv_count == 0 ||
               wv_count == 1 ||
               wv_count == 2 ||
               wv_count == 3 ||
               wv_count == 6 ||
               wv_count == 7;
    default:
        assert(false);
    }
}

APU::APU(bool bootrom_enabled) {
    registers.NR10 = 0x80;
    registers.NR11 = 0xBF;
    registers.NR12 = 0xF3;
    registers.NR13 = 0xFF; // no change
    registers.NR14 = 0xBF;
    registers.NR21 = 0x3F;
    registers.NR22 = 0x00;
    registers.NR23 = 0xFF; // no change
    registers.NR24 = 0xBF;
    registers.NR30 = 0x7F;
    registers.NR31 = 0xFF;
    registers.NR32 = 0x9F;
    registers.NR33 = 0xFF; // no change
    registers.NR34 = 0xBF;
    registers.NR40 = 0xFF; // no change
    registers.NR41 = 0xFF;
    registers.NR42 = 0x00;
    registers.NR43 = 0x00;
    registers.NR44 = 0xBF;
    registers.NR50 = 0x77;
    registers.NR51 = 0xF3;
    registers.NR52 = 0xF1; //TODO: 0xF0 on SGB
}
APU::~APU() {}

// The wiki Table
//Square 1: Sweep -> Timer -> Duty -> Length Counter -> Envelope -> Mixer
//Square 2:          Timer -> Duty -> Length Counter -> Envelope -> Mixer
//Wave:              Timer -> Wave -> Length Counter -> Volume   -> Mixer
//Noise:             Timer -> LFSR -> Length Counter -> Envelope -> Mixer

//My Function Table
//Square 1: fs_freq_sweep_clock -> timer_clock -> timer_clock -> length_counter_clock -> vol_env_clock -> ??
//Square 2:                        timer_clock -> timer_clock -> length_counter_clock -> vol_env_clock -> ??
//Wave:                            timer_clock -> ??          -> length_counter_clock -> vol_env_clock -> ??
//Noise:                           timer_clock -> ??          -> length_counter_clock -> vol_env_clock -> ??

/**
 * Simple Event Flow:
 *
 *  All audio is clocked by the Master Clock which is executed and timed by the CPU.
 * this is called upon by the tick function which is executed on every tick of the master clock.
 *
 *  This tick function runs both the Timer and the Frame Sequencer.
 *
 * ...
 *
 *  The audio library utilizes an asynchronous execution system whereupon a callback function
 * is used to queue audio for playback.
 *  In my system, this callback function will play the roll of the DAC, sampling the
 * inputs as often as it pleases to generate the proper audio.
 *  The "inputs" to the DAC will take the form of volatile variables so that they may be sampled
 * at any available rate.
 */

#define getFREQ(x,y) (((u16)(x))|(((y)&0x7)<<8))

void APU::tick() {

    tick_counter++;

    if(!(tick_counter % 65536)) { //64hz
        vol_env_clock(0);
    }

    if(!(tick_counter % 32768)) { //128hz
        freq_sweep_clock();
    }

    if(!(tick_counter % 16384)) { //256hz
        length_counter_clock(0);
    }

    if(!(tick_counter % 32)) {    //1317032 Hz
        //adding 1 here because it takes an extra cycle to reload the timer
        if(!channel_1.timer--) {
            channel_1.timer = ~getFREQ(registers.NR13, registers.NR14) + 1;
            timer_clock(1);
        }

        if(!channel_2.timer--) {
            channel_2.timer = ~getFREQ(registers.NR23, registers.NR24) + 1;
            timer_clock(2);
        }

        if(!channel_3.timer--) {
            channel_3.timer = ~getFREQ(registers.NR33, registers.NR34) + 1;
            timer_clock(2);
        }
    }

    if(!(tick_counter % 2)) {     //2097152 Hz
        timer_clock(4);
    }

    tick_counter %= 65536; //TODO
}

u8 APU::sample() {
    return normalize(channel_2.wav_out, channel_2.volume) >> 4;
}

void APU::timer_clock(u8 chan) {
    switch(chan) {
    case 1:
        channel_1.duty_counter++;
        channel_1.duty_counter %= 8;
        channel_1.wav_out = _duty_check((registers.NR11 & 0xC) >> 6, channel_1.duty_counter);
        break;
    case 2:
        channel_2.duty_counter++;
        channel_2.duty_counter %= 8;
        channel_1.wav_out = _duty_check((registers.NR21 & 0xC) >> 6, channel_2.duty_counter);
        break;
    case 3:

        break;
    case 4:
        if(!--channel_4.cfg_counter) {
            //reset counter
            channel_4.cfg_counter = getDivisor(registers.NR43 & 0x7) << (registers.NR43 >> 4);

            //XOR bottom 2 bits
            u8 b = Bit::test(channel_4.LFSR_REG, 0) ^ Bit::test(channel_4.LFSR_REG, 1);

            //right shift the register
            channel_4.LFSR_REG >>= 1;

            //set the high bit
            if(b) Bit::set(&channel_4.LFSR_REG, 14);
            else  Bit::reset(&channel_4.LFSR_REG, 14);

            //if 7 bit mode
            if (Bit::test(registers.NR43, 6)) {
                //set bit 6 too
                if(b) Bit::set(&channel_4.LFSR_REG, 6);
                else  Bit::reset(&channel_4.LFSR_REG, 6);
            }
        }
        channel_4.wav_out = Bit::test(channel_4.LFSR_REG, 0);
    }
}

void APU::length_counter_clock(u8 chan) {
    switch(chan) {
    case 0:
        length_counter_clock(1);
        length_counter_clock(2);
        length_counter_clock(3);
        length_counter_clock(4);
        break;
    case 1:
        if(!channel_1.length_counter && channel_1.enabled) {
            channel_1.length_counter--;
            if(!channel_1.length_counter) channel_1.enabled = false;
        }
        break;
    case 2:
        if(!channel_2.length_counter && channel_2.enabled) {
            channel_2.length_counter--;
            if(!channel_2.length_counter) channel_2.enabled = false;
        }
        break;
    case 3:
        if(!channel_3.length_counter && channel_3.enabled) {
            channel_3.length_counter--;
            if(!channel_3.length_counter) channel_3.enabled = false;
        }
        break;
    case 4:
        if(!channel_4.length_counter && channel_4.enabled) {
            channel_4.length_counter--;
            if(!channel_4.length_counter) channel_4.enabled = false;
        }
        break;
    }
}

void APU::freq_sweep_clock() {

}

void APU::freq_sweep_reset() {

}

void APU::vol_env_clock(u8 chan) {
    //TODO: does the period counter get reset both here and on a trigger?
    switch(chan) {
    case 0:
        vol_env_clock(1);
        vol_env_clock(2);
        vol_env_clock(3);
        vol_env_clock(4);
        break;
    case 1:
        if(channel_1.env_enabled && !channel_1.period_counter--) {
            if(registers.NR12 & 0x08) {
                channel_1.volume++;
                if(channel_1.volume != (channel_1.volume = std::min(channel_1.volume,(s8)0xF))) {
                    channel_1.env_enabled = false;
                }
            }
            else {
                channel_1.volume--;
                if(channel_1.volume != (channel_1.volume = std::max(channel_1.volume,(s8)0))) {
                    channel_1.env_enabled = false;
                }
            }
            //channel_1.period_counter = registers.NR12 & 0x07;
        }
        break;
    case 2:
        if(channel_2.env_enabled && !channel_2.period_counter--) {
            if(registers.NR22 & 0x08) {
                channel_2.volume++;
                if(channel_2.volume != (channel_2.volume = std::min(channel_2.volume,(s8)0xF))) {
                    channel_2.env_enabled = false;
                }
            }
            else {
                channel_2.volume--;
                if(channel_2.volume != (channel_2.volume = std::max(channel_2.volume,(s8)0))) {
                    channel_2.env_enabled = false;
                }
            }
            //channel_2.period_counter = registers.NR22 & 0x07;
        }
        break;
    case 4:
        if(channel_4.env_enabled && !channel_4.period_counter--) {
            if(registers.NR42 & 0x08) {
                channel_4.volume++;
                if(channel_4.volume != (channel_4.volume = std::min(channel_4.volume,(s8)0xF))) {
                    channel_4.env_enabled = false;
                }
            }
            else {
                channel_4.volume--;
                if(channel_4.volume != (channel_4.volume = std::max(channel_4.volume,(s8)0))) {
                    channel_4.env_enabled = false;
                }
            }
            //channel_4.period_counter = registers.NR42 & 0x07;
        }
        break;
    }
}

void APU::trigger(u8 chan) {
    // Channel is enabled (see length counter).
    // If length counter is zero, it is set to 64 (256 for wave channel).
    // Frequency timer is reloaded with period.
    // Volume envelope timer is reloaded with period.
    // Channel volume is reloaded from NRx2.

    switch(chan) {
    case 1:
        channel_1.enabled = true;
        if(!channel_1.length_counter) channel_1.length_counter = 64;
        //TODO: if the period in NR12 refers to the volume envelope, the what the hell is the period for the frequency timer?
        channel_1.timer = registers.NR12 & 0x7;
        channel_1.period_counter = registers.NR12 & 0x07;
        channel_1.volume = registers.NR12 >> 4;

        // Square 1's sweep does several things
        freq_sweep_reset();
    case 2:
    case 3:

        // Wave channel's position is set to 0 but sample buffer is NOT refilled.
        channel_3.wave_pos = 0;
    case 4:

        // Noise channel's LFSR bits are all set to 1.
        channel_4.LFSR_REG = 0xFF;
    }
}

u8 APU::read_reg(u8 loc) {
    switch(loc) {
    case NR10_REG:
        return registers.NR10 & NR10_READ_MASK;
    case NR11_REG:
        return registers.NR11 & NR11_READ_MASK;
    case NR12_REG:
        return registers.NR12 & NR12_READ_MASK;
    case NR13_REG:
        return registers.NR13 & NR13_READ_MASK;
    case NR14_REG:
        return registers.NR14 & NR14_READ_MASK;

    case NR21_REG:
        return registers.NR21 & NR21_READ_MASK;
    case NR22_REG:
        return registers.NR22 & NR22_READ_MASK;
    case NR23_REG:
        return registers.NR23 & NR23_READ_MASK;
    case NR24_REG:
        return registers.NR24 & NR24_READ_MASK;

    case NR30_REG:
        return registers.NR30 & NR30_READ_MASK;
    case NR31_REG:
        return registers.NR31 & NR31_READ_MASK;
    case NR32_REG:
        return registers.NR32 & NR32_READ_MASK;
    case NR33_REG:
        return registers.NR33 & NR33_READ_MASK;
    case NR34_REG:
        return registers.NR34 & NR34_READ_MASK;

    case NR41_REG:
        return registers.NR41 & NR41_READ_MASK;
    case NR42_REG:
        return registers.NR42 & NR42_READ_MASK;
    case NR43_REG:
        return registers.NR43 & NR43_READ_MASK;
    case NR44_REG:
        return registers.NR44 & NR44_READ_MASK;

    case NR50_REG:
        return registers.NR50 & NR50_READ_MASK;
    case NR51_REG:
        return registers.NR51 & NR51_READ_MASK;
    case NR52_REG:
        return registers.NR52 & NR52_READ_MASK;
    }
}

void APU::write_reg(u8 loc, u8 data) {
    switch(loc) {

    //channel 1 registers
    case NR10_REG:
        registers.NR10 = data & NR10_WRITE_MASK;
        break;
    case NR11_REG:
        registers.NR11 = data & NR11_WRITE_MASK;
        channel_1.length_counter = 64 - (data & 0x3F);
        break;
    case NR12_REG:
        registers.NR12 = data & NR12_WRITE_MASK;
        channel_1.volume = data >> 4;
        channel_1.period_counter = data & 0x07;
        break;
    case NR13_REG:
        registers.NR13 = data & NR13_WRITE_MASK;
        break;
    case NR14_REG:
        registers.NR14 = data & NR14_WRITE_MASK;
        //writing to this register triggers it
        if(data & 0x80) trigger(1);
        break;

    //channel 2 registers
    case NR21_REG:
        registers.NR21 = data & NR21_WRITE_MASK;
        channel_2.length_counter = 64 - (data & 0x3F);
        break;
    case NR22_REG:
        registers.NR22 = data & NR22_WRITE_MASK;
        break;
    case NR23_REG:
        registers.NR23 = data & NR23_WRITE_MASK;
        break;
    case NR24_REG:
        registers.NR24 = data & NR24_WRITE_MASK;
        if(data & 0x80) trigger(2);
        break;

    //channel 3 registers
    case NR30_REG:
        registers.NR30 = data & NR30_WRITE_MASK;
        break;
    case NR31_REG:
        registers.NR31 = data & NR31_WRITE_MASK;
        channel_3.length_counter = 256 - data;
        break;
    case NR32_REG:
        registers.NR32 = data & NR32_WRITE_MASK;
        break;
    case NR33_REG:
        registers.NR33 = data & NR33_WRITE_MASK;
        break;
    case NR34_REG:
        registers.NR34 = data & NR34_WRITE_MASK;
        if(data & 0x80) trigger(3);
        break;

    //channel 4 registers
    case NR41_REG:
        registers.NR41 = data & NR41_WRITE_MASK;
        channel_4.length_counter = 64 - (data & 0x3F);
        break;
    case NR42_REG:
        registers.NR42 = data & NR42_WRITE_MASK;
        break;
    case NR43_REG:
        //registers.NR43 = data & NR43_WRITE_MASK;
        channel_4.cfg_counter = channel_4.cfg_timer = getDivisor(registers.NR43 & 0x7) << (registers.NR43 >> 4);
        channel_4.LFSR_7bit = (bool)Bit::test(registers.NR43, 3);
        break;
    case NR44_REG:
        registers.NR44 = data & NR44_WRITE_MASK;
        if(data & 0x80) trigger(4);
        break;

    case NR50_REG:
        registers.NR50 = data & NR50_WRITE_MASK;
        break;
    case NR51_REG:
        registers.NR51 = data & NR51_WRITE_MASK;
        break;
    case NR52_REG:
        registers.NR52 = data & NR52_WRITE_MASK;
        break;
    }
}

u8 APU::read_wavram(u8 loc) {
    if(loc >= WAVRAM_LEN) return wav_ram[loc];
    return 0;
}

void APU::write_wavram(u8 loc, u8 data) {
    if(loc < WAVRAM_LEN) wav_ram[loc] = data;
}