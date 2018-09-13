#include "audio.hpp"

#include <iostream>

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
    }
}


void _audio_callback(void *user_data, Uint8 *raw_buffer, int bytes) {
    //TODO
}

Sound_Controller::Sound_Controller() {
    if(SDL_Init(SDL_INIT_AUDIO))
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;

    SDL_AudioSpec desired;
    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_S16;
    desired.channels = 2;
    desired.samples = 2048;
    desired.callback = _audio_callback;
    //want1.userdata = &sample_nr;

    SDL_AudioSpec have;
    if(SDL_OpenAudio(&desired, &have))
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;

    if(desired.format != have.format)
        std::cerr << "Failed to get the desired AudioSpec" << std::endl;
}

// The wiki Table
//Square 1: Sweep -> Timer -> Duty -> Length Counter -> Envelope -> Mixer
//Square 2:          Timer -> Duty -> Length Counter -> Envelope -> Mixer
//Wave:              Timer -> Wave -> Length Counter -> Volume   -> Mixer
//Noise:             Timer -> LFSR -> Length Counter -> Envelope -> Mixer

//My Function Table
//Square 1: fs_freq_sweep_clock -> timer_clock -> timer_clock -> fs_length_counter_clock -> fs_vol_env_clock -> ??
//Square 2:                        timer_clock -> timer_clock -> fs_length_counter_clock -> fs_vol_env_clock -> ??
//Wave:                            timer_clock -> ??          -> fs_length_counter_clock -> fs_vol_env_clock -> ??
//Noise:                           timer_clock -> ??          -> fs_length_counter_clock -> fs_vol_env_clock -> ??

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
 * at any available rate. as the audio callback takes a signed 16 bit number representing the
 * the height of the waveform at the current time, this should work nicely.
 *  A scaling function will have to be used to convert the output from what i assume 
 * is a 4bit space to a 16bit space to ensure audio doesn't seem overly quiet.
 */

#define getFREQ(x,y) (((u16)(x))|(((y)&0x7)<<8))

void Sound_Controller::tick() {
    if(!channel_1.timer--) {
        channel_1.timer = getFREQ(registers.NR13, registers.NR14);
        timer_clock(1);
    }

    if(!channel_2.timer--) {
        channel_2.timer = getFREQ(registers.NR23, registers.NR24);
        timer_clock(2);
    }

    if(!channel_2.timer--) {
        channel_2.timer = getFREQ(registers.NR33, registers.NR34);
        timer_clock(2);
    }

    if(!channel_4.timer--) {
        channel_2.timer = getFREQ(registers.NR33, registers.NR34);
        timer_clock(4);
    }

    tick_counter++;
    if(!(tick_counter % 8192)) {
        fs_counter++;
        switch(fs_counter %= 8) {
        case 0: //clock length_counter
            fs_length_counter_clock(0);
            break;
        case 2: //clock length_counter, sweep
            fs_length_counter_clock(0);
            fs_freq_sweep_clock();
            break;
        case 4: //clock length_counter
            fs_length_counter_clock(0);
            break;
        case 6: //clock length_counter, sweep
            fs_length_counter_clock(0);
            fs_freq_sweep_clock();
            break;
        case 7: //clock vol_env
            fs_vol_env_clock(0);
            break;
        default: break;
        }
    }
}

void Sound_Controller::timer_clock(u8 chan) {
    switch(chan) {
    case 1:
        channel_1.duty_counter++;
        _duty_check(registers.NR11 & 0xC, channel_1.duty_counter %= 8);
        break;
    case 2:
        channel_2.duty_counter++;
        _duty_check(registers.NR11 & 0xC, channel_1.duty_counter %= 8);
    case 3:
    case 4:
    }
}

void Sound_Controller::fs_length_counter_clock(u8 chan) {
    switch(chan) {
    case 0:
        fs_length_counter_clock(1);
        fs_length_counter_clock(2);
        fs_length_counter_clock(3);
        fs_length_counter_clock(4);
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

void Sound_Controller::fs_freq_sweep_clock() {

}

void Sound_Controller::freq_sweep_reset() {

}

void Sound_Controller::fs_vol_env_clock(u8 chan) {
    //TODO: does the period counter get reset both here and on a trigger?
    switch(chan) {
    case 0:
        fs_vol_env_clock(1);
        fs_vol_env_clock(2);
        fs_vol_env_clock(3);
        fs_vol_env_clock(4);
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

void Sound_Controller::trigger(u8 chan) {
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

u8 Sound_Controller::read_reg(u8 loc) {
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

void Sound_Controller::write_reg(u8 loc, u8 data) {
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

u8 Sound_Controller::read_wavram(u8 loc) {
    if(loc >= WAVRAM_LEN) return wav_ram[loc];
    return 0;
}
void Sound_Controller::write_wavram(u8 loc, u8 data) {
    if(loc < WAVRAM_LEN) wav_ram[loc] = data;
}