#pragma once

#include "defs.hpp"
#include "cpu.hpp"
#include "io.hpp"
#include "ppu.hpp"

#include "util/file.hpp"
#include "util/types/ringbuffer.hpp"

struct breakpoint_exception {
    breakpoint_exception() {}
};

namespace Silver {

class Core {
public:
    static constexpr u32 native_width = 160;
    static constexpr u32 native_height = 144;
    static constexpr u32 native_pixel_count = native_width * native_height;

    Core(Silver::File *rom, Silver::File *bootrom = nullptr, gb_device_t device = device_GBC);
    ~Core();


    void init_thread(bool paused = true);
    void pause_thread();
    bool thread_paused();
    void resume_thread();
    void stop_thread();
    void run_thread();

    void tick_once();
    void tick_instr();
    void tick_frame();
    void tick_delta_or_frame();
//    void tick_audio_buffer(u8* buf, int buf_len);

    bool is_frame_ready();

    void set_input_state(Joypad::button_states_t const& state);
    void do_audio_callback(float *buff, int copy_cnt);

    CPU::registers_t getRegistersFromCPU();
    Memory::io_registers_t getregistersfromIO();
    u8 getByteFromIO(u16 addr);
    u8 const* getScreenBuffer();

    std::vector<u8> getOAMEntry(int index);
    void getBGBuffer(u8 *buf);
    void getWNDBuffer(u8 *buf);

    void set_bp(u16 bp, bool en = false);
    u16 get_bp();
    void set_bp_active(bool en);
    bool get_bp_active();

private:
    Memory *mem;
    Cartridge *cart;
    APU *apu;
    PPU *ppu;
    Joypad *joy;
    IO_Bus *io;
    CPU *cpu;

    gb_device_t device;

    u8 *screen_buffer = nullptr;
    bool frame_ready = false;
    jnk0le::Ringbuffer<std::vector<float>,4> *audio_queue = nullptr;
    std::vector<float> audio_vector;

    u16 breakpoint = 0;
    bool bp_active = false;
};

} // namespace Silver