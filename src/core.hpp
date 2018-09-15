#pragma once

#include "file.hpp"
#include "cfg.hpp"
#include "cpu.hpp"
#include "io.hpp"

#include <thread>
#include <atomic>

class GB_Core {
public:
    GB_Core(File_Interface *rom, Configuration *cfg);
    ~GB_Core();

    void start();
    void pause();
    void resume();
    void stop();
private:
    Cartridge *cart;
    IO_Bus *io;
    CPU *cpu;


    std::atomic<bool>
            thread_killed = ATOMIC_VAR_INIT(false),
            thread_paused = ATOMIC_VAR_INIT(false);

    std::thread core_thread;


    void loop();
};