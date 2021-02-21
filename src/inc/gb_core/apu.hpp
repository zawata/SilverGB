#pragma once

#include "defs.hpp"

#include "util/bit.hpp"
#include "util/ints.hpp"

#define WAVRAM_LEN 16

class APU {
public:
    APU(bool bootrom_enabled);
    ~APU();

    void tick();

    u8 sample();

    u8 read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    u8 read_wavram(u8 loc);
    void write_wavram(u8 loc, u8 data);

private:
    u32 tick_counter;

    struct {
        bool enabled;
        s8 volume;
        u16 timer;
        int length_counter;

        u8 duty_counter;

        bool wav_out;

        //envelope
        bool env_enabled;
        u8 period_counter;
    } channel_1;

    struct {
        bool enabled;
        s8 volume;
        u16 timer;
        int length_counter;

        u8 duty_counter;

        bool wav_out;

        //envelope
        bool env_enabled;
        u8 period_counter;
    } channel_2;

    struct {
        bool enabled;
        s8 volume;
        u16 timer;
        int length_counter;

        u8 wave_pos;
    } channel_3;

    struct {
        bool enabled;

        //CFG timer and PRNG
        u32  cfg_counter;
        u32  cfg_timer;
        u16 LFSR_REG;
        bool LFSR_7bit; 

        int length_counter;
        s8 volume;
        u8 period_counter;
        bool env_enabled;

        bool wav_out;


    } channel_4;

    struct __registers {
        u8 NR10;
        u8 NR11;
        u8 NR12;
        u8 NR13;
        u8 NR14;

        u8 NR21;
        u8 NR22;
        u8 NR23;
        u8 NR24;

        u8 NR30;
        u8 NR31;
        u8 NR32;
        u8 NR33;
        u8 NR34;

        u8 NR40;
        u8 NR41;
        u8 NR42;
        u8 NR43;
        u8 NR44;

        u8 NR50;
        u8 NR51;
        u8 NR52;
    } registers;

    u8 wav_ram[WAVRAM_LEN];

    inline u8 getDivisor(u8 i) { return (u8[8]){8, 16, 32, 48, 64, 80, 96, 112}[i & 0x7]; }

    void timer_clock(u8 chan);
    void length_counter_clock(u8 chan);
    void freq_sweep_clock();
    void freq_sweep_reset();
    void vol_env_clock(u8 chan);
    void trigger(u8 chan);
};