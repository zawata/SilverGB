#include <cassert>
#include <cstring>

#include <chrono>

#include <nowide/iostream.hpp>

#include "gb_core/defs.hpp"
#include "gb_core/apu.hpp"

#include "util/bit.hpp"
#include "util/util.hpp"

#define NUETRAL ( 0 )

enum {
    ALL_CHANNELS = 0,
    CHANNEL_1    = 1,
    CHANNEL_2    = 2,
    CHANNEL_3    = 3,
    CHANNEL_4    = 4,
};

inline bool _duty_check(u8 duty, u8 wv_count) {
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
        return !_duty_check(1,wv_count);
    }
    assert(false);
    return false;
}

APU::APU(bool bootrom_enabled) {
    init_core();
    registers = { 0 };
}
APU::~APU() {}

void APU::init_core() {
    memset(&channel_1, 0 , sizeof(channel_1));
    memset(&channel_2, 0 , sizeof(channel_2));
    memset(&channel_3, 0 , sizeof(channel_3));
    memset(&channel_4, 0 , sizeof(channel_4));
}

// The wiki Table
//Square 1: Sweep -> Timer -> Duty -> Length Counter -> Envelope -> Mixer
//Square 2:          Timer -> Duty -> Length Counter -> Envelope -> Mixer
//Wave:              Timer -> Wave -> Length Counter -> Volume   -> Mixer
//Noise:             Timer -> LFSR -> Length Counter -> Envelope -> Mixer

//My Function Table
//Square 1: freq_sweep_clock -> timer_clock (duty) -> length_counter_clock -> vol_env_clock -> sample
//Square 2:                     timer_clock (duty) -> length_counter_clock -> vol_env_clock -> sample
//Wave:                         timer_clock (wave) -> length_counter_clock -> vol_env_clock -> sample
//Noise:                        timer_clock (LFSR) -> length_counter_clock -> vol_env_clock -> sample

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
 *  In my system, this callback function will play the role of the DAC, sampling the
 * inputs as often as it pleases to generate the proper audio.
 *  The "inputs" to the DAC will take the form of volatile variables so that they may be sampled
 * at any available rate.
 */
bool tick_print_enabled = false;
void APU::tick() {
    if(!(tick_counter % 8192)) {
        switch(frame_sequence_cntr) {
        case 0:
            length_counter_clock(ALL_CHANNELS);
            break;
        case 1:
            break;
        case 2:
            length_counter_clock(ALL_CHANNELS);
            freq_sweep_clock();
            break;
        case 3:
            break;
        case 4:
            length_counter_clock(ALL_CHANNELS);
            break;
        case 5:
            break;
        case 6:
            length_counter_clock(ALL_CHANNELS);
            freq_sweep_clock();
            break;
        case 7:
            vol_env_clock(ALL_CHANNELS);
            break;
        }

        frame_sequence_cntr++;
        frame_sequence_cntr %= 8;
    }

    //1048576
    if(!(tick_counter % 4)) {
        timer_clock(1);
        timer_clock(2);
        timer_clock(3);
    }

    //2097152 Hz
    if(!(tick_counter % 2)) {
        timer_clock(4);
    }

    tick_counter++;
    tick_counter %= 65536;
}

void mix_channel(bool wave, u8 volume, float *out) {
    #define _mix(a,b) (((a)+(b))/2)
    //TODO: figure out the proper range to use for the "DAC"
    // if the range is from -1.0 to 1.0 then the silence value would need to be -1.0
    // otherwise the volume values 0-7 would be essentially equivalent to 8-F
    // #define _normalize(u4val) (((u4val) / 7.5f) - 1.0f)
    #define _normalize(u4val) ((u4val) / 15.0f)

    *out = _mix((wave) ? _normalize(volume & 0xF) : 0.0f, *out);
}

void APU::sample(float *left, float *right) {
    *left = 0.0f;
    *right = 0.0f;
    if( snd_en() ) {
        if(channel_1.enabled) {
            if(ch1_L_dac_en()) mix_channel(channel_1.wav_out, channel_1.volume, left);
            if(ch1_R_dac_en()) mix_channel(channel_1.wav_out, channel_1.volume, right);
        }

        if(channel_2.enabled) {
            if(ch2_L_dac_en()) mix_channel(channel_2.wav_out, channel_2.volume, left);
            if(ch2_R_dac_en()) mix_channel(channel_2.wav_out, channel_2.volume, right);
        }

        // if(channel_3.enabled) {
            // if(ch3_L_dac_en()) mix_channel(channel_3.wav_out, channel_3.volume, left);
            // if(ch3_R_dac_en()) mix_channel(channel_3.wav_out, channel_3.volume, right);
        // }

        if(channel_4.enabled) {
            if(ch4_L_dac_en()) mix_channel(channel_4.wav_out, channel_4.volume, left);
            if(ch4_R_dac_en()) mix_channel(channel_4.wav_out, channel_4.volume, right);
        }
    }

    //TODO: temporary, want to make this configurable and put it in core
    *left /= 4;
    *right /= 4;
}

void APU::timer_clock(u8 chan) {
    switch(chan) {
    case CHANNEL_1:
        if(channel_1.timer == 0) {
            channel_1.timer = 2048 - ch1_freq();

            channel_1.duty_counter++;
            channel_1.duty_counter %= 8;
            channel_1.wav_out = _duty_check(ch1_wav_patt_duty(), channel_1.duty_counter);
        }
        else {
            channel_1.timer--;
        }
        break;
    case CHANNEL_2:
        if(channel_2.timer == 0) {
            channel_2.timer = 2048 - ch2_freq();

            channel_2.duty_counter++;
            channel_2.duty_counter %= 8;
            channel_2.wav_out = _duty_check(ch2_wav_patt_duty(), channel_2.duty_counter);
        }
        else {
            channel_2.timer--;
        }
        break;
    case CHANNEL_4:
        if(channel_4.cfg_counter == 0) {
            //this is left shifted by 1 because the output line of the counter is supposed to be inverted on TC, not reloaded
            //easiest fix is to just double the clock timer and still reload on rising signals
            channel_4.cfg_counter = (ch4_div_ratio() << 1) + 1;

            if(channel_4.shift_clock_cntr == 0) {
                channel_4.shift_clock_cntr = 0;
                Bit::set(&channel_4.shift_clock_cntr, ch4_shft_freq());

                u8 r = Bit::test(channel_4.LFSR_REG, 0) ^ Bit::test(channel_4.LFSR_REG, 1);

                channel_4.LFSR_REG >>= 1;
                if(ch4_reg_width()) {
                    if(r) Bit::set(&channel_4.LFSR_REG, 6);
                    else  Bit::reset(&channel_4.LFSR_REG, 6);
                }

                if(r) Bit::set(&channel_4.LFSR_REG, 14);
                else  Bit::reset(&channel_4.LFSR_REG, 14);

                //output is inverted!
                channel_4.wav_out = !Bit::test(channel_4.LFSR_REG, 0);
            } else {
                channel_4.shift_clock_cntr--;
            }
        } else {
            channel_4.cfg_counter--;
        }
    }
}

void APU::length_counter_clock(u8 chan) {
    switch(chan) {
    case ALL_CHANNELS:
        length_counter_clock(CHANNEL_1);
        length_counter_clock(CHANNEL_2);
        length_counter_clock(CHANNEL_3);
        length_counter_clock(CHANNEL_4);
        break;
    case CHANNEL_1:
        if(channel_1.length_counter && channel_1.enabled) {
            channel_1.length_counter--;
            if(!channel_1.length_counter) {
                channel_1.enabled = false;
            }
        }
        break;
    case CHANNEL_2:
        if(!channel_2.length_counter && channel_2.enabled) {
            channel_2.length_counter--;
            if(!channel_2.length_counter) channel_2.enabled = false;
        }
        break;
    case CHANNEL_3:
        if(!channel_3.length_counter && channel_3.enabled) {
            channel_3.length_counter--;
            if(!channel_3.length_counter) channel_3.enabled = false;
        }
        break;
    case CHANNEL_4:
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
    channel_1.clock_volume_envelope();
    channel_2.clock_volume_envelope();
    channel_4.clock_volume_envelope();
}

void APU::trigger(u8 chan) {
    // Channel is enabled (see length counter).
    // If length counter is zero, it is set to 64 (256 for wave channel).
    // Frequency timer is reloaded with period.
    // Volume envelope timer is reloaded with period.
    // Channel volume is reloaded from NRx2.

    switch(chan) {
    case CHANNEL_1:
        channel_1.enabled = true;
        if(channel_1.length_counter == 0) channel_1.length_counter = 64;
        channel_1.timer = 2048 - ch1_freq();

        channel_1.period_counter = ch1_env_swp_prd();
        channel_1.increment = ch1_env_dir();
        channel_1.env_enabled = channel_1.period_counter > 0;

        channel_1.volume = ch1_init_vol_env();
        // Square 1's sweep does several things
        freq_sweep_reset();
        break;
    case CHANNEL_2:
        channel_2.enabled = true;
        if(channel_2.length_counter == 0) channel_2.length_counter = 64;
        channel_2.timer = 2048 - ch2_freq();

        channel_2.period_counter = ch2_env_swp_prd();
        channel_2.increment = ch2_env_dir();
        channel_2.env_enabled = channel_2.period_counter > 0;

        channel_2.volume = ch2_init_vol_env();
        break;
    case CHANNEL_3:
        channel_3.enabled = true;
        if(channel_3.length_counter == 0) channel_3.length_counter = 256;

        // Wave channel's position is set to 0 but sample buffer is NOT refilled.
        channel_3.wave_pos = 0;
        break;
    case CHANNEL_4:
        channel_4.enabled = true;
        if(channel_4.length_counter == 0) channel_4.length_counter = 64;

        channel_4.period_counter = ch4_env_swp_prd();
        channel_4.increment = ch4_env_dir();
        channel_4.env_enabled = channel_4.period_counter > 0;

        channel_4.volume = ch1_init_vol_env();
        // static int trigger_count = 0;

        // if(trigger_count) tick_print_enabled = true;
        // trigger_count++;

        // Noise channel's LFSR bits are all set to 1.
        channel_4.LFSR_REG = 0xFFFF;
        break;
    }
}

//TODO: all of this can be moved to the IO reg-io wrappers
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

    case NR20_REG:
        return NR20_DEFAULTS;
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

    case NR40_REG:
        return NR40_DEFAULTS;
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
    default:
        nowide::cerr << "bad apu reg read: " << loc << std::endl;
        return 0xFF;
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
        channel_1.length_counter = ~ch1_snd_len() + 1;
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
        if(data & 0x80) trigger(1);
        break;

    //channel 2 registers
    case NR20_REG:
        break;

    case NR21_REG:
        registers.NR21 = data & NR21_WRITE_MASK;
        channel_2.length_counter = ~ch2_snd_len() + 1;
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
    case NR40_REG:
        break;

    case NR41_REG:
        registers.NR41 = data & NR41_WRITE_MASK;
        channel_4.length_counter = 64 - (data & 0x3F);
        break;
    case NR42_REG:
        registers.NR42 = data & NR42_WRITE_MASK;
        break;
    case NR43_REG:
        registers.NR43 = data & NR43_WRITE_MASK;
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
    if(loc < WAVRAM_LEN) return wav_ram[loc];
    return 0;
}

void APU::write_wavram(u8 loc, u8 data) {
    if(loc < WAVRAM_LEN) wav_ram[loc] = data;
}