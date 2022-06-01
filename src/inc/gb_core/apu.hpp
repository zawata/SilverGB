#pragma once

#include "defs.hpp"

#include "util/bit.hpp"
#include "util/ints.hpp"

#define WAVRAM_LEN 16

#define protected_constructor(classname) \
    protected: \
        classname() {}; \
    public: \

struct _generic_channel {
    protected_constructor(_generic_channel)

    bool enabled;
    s8 volume;
};

struct _volume_envelope : public _generic_channel {
    protected_constructor(_volume_envelope)

    void clock_volume_envelope() {
        if(this->env_enabled) {
            if(this->period_counter == 0) {
                if(this->increment) {
                    if(this->volume == 0xF) {
                        this->env_enabled = false;
                    } else {
                        this->volume++;
                    }
                } else {
                    if (this->volume == 0) {
                        this->env_enabled = false;
                    } else {
                        this->volume--;
                    }
                }
            } else {
                this->period_counter--;
            }
        }
    }

    bool env_enabled;
    u8 period_counter;
    bool increment;
};

struct _programmable_timer {
    protected_constructor(_programmable_timer)

    u16 timer;
};

struct _configurable_timer {
    protected_constructor(_configurable_timer)

    u32  cfg_counter;
    u32  shift_clock_cntr;
};

struct _length_counter {
    protected_constructor(_length_counter)

    int length_counter;
};

struct _duty_cycle_generator {
    protected_constructor(_duty_cycle_generator)

    u8 duty_counter;
    bool wav_out;
};

struct _prn_generator {
    protected_constructor(_prn_generator)

    u16  LFSR_REG;
    bool wav_out;
};

class IO_Bus;

class APU {
friend IO_Bus;
public:
    explicit APU(bool bootrom_enabled);
    ~APU();

    void tick();

    void sample(float *left, float *right);

    u8 read_reg(u8 loc);
    void write_reg(u8 loc, u8 data);

    u8 read_wavram(u8 loc);
    void write_wavram(u8 loc, u8 data);

private:
    u32 tick_counter = 0;
    u16 frame_sequence_cntr = 0;

    struct :
        public _volume_envelope,
        public _programmable_timer,
        public _length_counter,
        public _duty_cycle_generator
    {} channel_1;

    #define reg(X) (registers.X)
    inline u8  ch1_sweep_rt()       { return (reg(NR10) >> 4) & 7; }
    inline u8  ch1_sweep_dir()      { return Bit::test(reg(NR10), 3); }
    inline u8  ch1_sweep_shft_amt() { return reg(NR10) & 3; }

    inline u8  ch1_wav_patt_duty()  { return reg(NR11) >> 6; }
    inline u8  ch1_snd_len()        { return reg(NR11) & 0x3F; }

    inline u8  ch1_init_vol_env()   { return reg(NR12) >> 4; }
    inline u8  ch1_env_dir()        { return Bit::test(reg(NR12), 3); }
    inline u8  ch1_env_swp_prd()    { return reg(NR12) & 0x7; }

    inline u16 ch1_freq()           { return reg(NR13) | ((u16)(reg(NR14) & 0x7) << 8); }

    inline u8  ch1_len_cntr_en()    { return reg(NR14) >> 6; }
    #undef reg

    /**
     * Channel 2
     **/
    struct :
        public _volume_envelope,
        public _programmable_timer,
        public _length_counter,
        public _duty_cycle_generator
    {} channel_2;

    #define reg(X) (registers.X)
    inline u8  ch2_wav_patt_duty()  { return reg(NR21) >> 6; }
    inline u8  ch2_snd_len()        { return reg(NR21) & 0x3F; }

    inline u8  ch2_init_vol_env()   { return reg(NR22) >> 4; }
    inline u8  ch2_env_dir()        { return Bit::test(reg(NR22), 3); }
    inline u8  ch2_env_swp_prd()    { return reg(NR22) & 0x7; }

    inline u16 ch2_freq()           { return reg(NR23) | ((u16)(reg(NR24) & 0x7) << 8); }

    inline u8  ch2_len_cntr_en()    { return reg(NR24) >> 6; }
    #undef reg

    /**
     * Channel 3
     **/
    struct :
        public _generic_channel,
        public _programmable_timer,
        public _length_counter
    {
        u8 wave_pos;
    } channel_3;

    #define reg(X) (registers.X)
    inline u8  ch3_mstr_en()        { return Bit::test(reg(NR30), 7); }

    inline u8  ch3_snd_len()        { return reg(NR31); }

    inline u8  ch3_vol()            { return (reg(NR32) >> 5) & 3; }

    inline u16 ch3_freq()           { return reg(NR33) | ((u16)(reg(NR34) & 0x7) << 8); }

    inline u8  ch3_len_cntr_en()    { return reg(NR34) >> 6; }
    #undef reg

    /**
     * channel 4
     **/
    struct _channel_4 :
        public _volume_envelope,
        public _configurable_timer,
        public _length_counter,
        public _prn_generator
    {} channel_4;

    #define reg(X) (registers.X)
    inline u8  ch4_wav_patt_duty()  { return reg(NR41) >> 6; }
    inline u8  ch4_snd_len()        { return reg(NR41) & 0x3F; }

    inline u8  ch4_init_vol_env()   { return reg(NR42) >> 4; }
    inline u8  ch4_env_dir()        { return Bit::test(reg(NR42), 3); }
    inline u8  ch4_env_swp_prd()    { return reg(NR42) & 0x7; }

    inline u8  ch4_shft_freq()      { return (reg(NR43) >> 4) & 0xF; }
    inline u8  ch4_reg_width()      { return Bit::test(reg(NR43), 3); }
    inline u8  ch4_div_ratio()      { return reg(NR43) & 7; }

    inline u8  ch4_len_cntr_en()    { return Bit::test(reg(NR44), 6); }
    #undef reg

    /**
     *Control
     **/
    #define reg(X) (registers.X)
    inline bool ch1_L_dac_en() { return Bit::test(reg(NR51), 4); }
    inline bool ch1_R_dac_en() { return Bit::test(reg(NR51), 0); }
    inline bool ch2_L_dac_en() { return Bit::test(reg(NR51), 5); }
    inline bool ch2_R_dac_en() { return Bit::test(reg(NR51), 1); }
    inline bool ch3_L_dac_en() { return Bit::test(reg(NR51), 6); }
    inline bool ch3_R_dac_en() { return Bit::test(reg(NR51), 2); }
    inline bool ch4_L_dac_en() { return Bit::test(reg(NR51), 7); }
    inline bool ch4_R_dac_en() { return Bit::test(reg(NR51), 3); }

    inline bool snd_en()       { return Bit::test(reg(NR52), 7); }
    #undef reg

    struct __registers {
        u8 NR10;
        u8 NR11;
        u8 NR12;
        u8 NR13;
        u8 NR14;

        u8 NR20; // unused
        u8 NR21;
        u8 NR22;
        u8 NR23;
        u8 NR24;

        u8 NR30;
        u8 NR31;
        u8 NR32;
        u8 NR33;
        u8 NR34;

        u8 NR40; // unused
        u8 NR41;
        u8 NR42;
        u8 NR43;
        u8 NR44;

        u8 NR50;
        u8 NR51;
        u8 NR52;
    } registers;

    u8 wav_ram[WAVRAM_LEN];

    inline u8 getDivisor(u8 i) {
        const u8 t[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
        return t[i & 0x7];
    }

    void timer_clock(u8 chan);
    void length_counter_clock(u8 chan);
    void freq_sweep_clock();
    void freq_sweep_reset();
    void vol_env_clock(u8 chan);
    void trigger(u8 chan);
};