#pragma once

#include "util/file.hpp"
#include "gb_core/cpu.hpp"
#include "gb_core/io.hpp"
#include "gb_core/ppu.hpp"

#include <thread>
#include <atomic>

struct breakpoint_exception {
    breakpoint_exception() {}
};

class GB_Core {
public:
    GB_Core(Silver::File *rom, Silver::File *bootrom);
    ~GB_Core();


    void init_thread(bool paused = true);
    void pause_thread();
    bool thread_paused();
    void resume_thread();
    void stop_thread();
    void run_thread();

    void tick_once();
    void tick_instr();
    void tick_frame();
    void tick_audio_buffer(u8* buf, int buf_len);

    void set_input_state(Input_Manager::button_states_t const& state);

    CPU::registers_t getRegistersFromCPU();
    IO_Bus::io_registers_t getregistersfromIO();
    u8 getByteFromIO(u16 addr);
    u8 const* getScreenBuffer();

    std::vector<u8> getOAMEntry(int index);
    void getBGBuffer(u8 *buf);
    void getWNDBuffer(u8 *buf);

    void set_bp(u16 bp, bool en = false);
    u16 get_bp();
    void set_bp_active(bool en);
    bool get_bp_active();

    //void dump_io_registers();

private:
    Cartridge *cart;
    IO_Bus *io;
    APU *apu;
    CPU *cpu;
    PPU *vpu;

    u8 *screen_buffer = NULL;

    u16 breakpoint = 0;
    bool bp_active = false;

    std::thread core_thread;
};