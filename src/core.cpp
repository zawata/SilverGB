#include "core.hpp"

#include <thread>
#include <chrono>

GB_Core::GB_Core(File_Interface *rom, Configuration *cfg) :
cart(new Cartridge(rom)),
io(new IO_Bus(cart, cfg)),
cpu(new CPU(io)) {}

GB_Core::~GB_Core() {
    delete cpu;
    delete io;
    delete cart;
}

void GB_Core::start() {
    thread_killed = false;
    core_thread = std::thread(&GB_Core::loop, this);
}

void GB_Core::pause() {
    thread_paused = true;
}

void GB_Core::resume() {
    thread_paused = false;
}

void GB_Core::stop() {
    thread_killed = true;
    core_thread.join();
}

void GB_Core::loop() {
    while(!thread_killed) {
        while(thread_paused);
        auto start = std::chrono::high_resolution_clock::now();

        //every tick should take exactly 238 nanoseconds
        cpu->clock_pulse();

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::this_thread::sleep_for(std::chrono::nanoseconds(238)-duration);
    }
}