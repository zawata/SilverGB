#pragma once

#include "file.hpp"
#include "cfg.hpp"
#include "cpu.hpp"
#include "io.hpp"
#include "video.hpp"

#include <thread>
#include <atomic>

class GB_Core {
public:
    GB_Core(File_Interface *rom, Configuration *cfg);
    ~GB_Core();

    void start(bool paused = false);
    void pause();
    bool paused();
    void resume();
    void stop();
    void tick();

    void core_tick();

    CPU::registers_t getRegistersFromCPU();
    u8 getByteFromIO(u16 addr);

private:
    Cartridge *cart;
    IO_Bus *io;
    CPU *cpu;
    Video_Controller *vpu;

    std::thread core_thread;


    void loop();
};