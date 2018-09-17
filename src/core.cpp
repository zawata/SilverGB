#include "core.hpp"

#include <thread>
#include <chrono>

static std::atomic<bool> loop_paused;

GB_Core::GB_Core(File_Interface *rom, Configuration *cfg) {
    cart = new Cartridge(rom);
    io = new IO_Bus(cart, cfg);
    cpu = new CPU(io, cfg);
}

GB_Core::~GB_Core() {
    delete cpu;
    delete io;
    delete cart;
}

// void GB_Core::fetch_registers() {
//     thread_paused = true;
//     //check the "mutex"
//     while(loop_paused);
//     cpu->fetch_registers();
//     thread_paused = false;;
// }

void GB_Core::start(bool paused) {
    thread_killed = false;
    if(paused) thread_paused = false;
    core_thread = std::thread(&GB_Core::loop, this);
}

void GB_Core::pause() {
    thread_paused = true;
}

bool GB_Core::paused() {
    return thread_paused || thread_killed; }

void GB_Core::resume() {
    thread_paused = false;
}

void GB_Core::stop() {
    thread_killed = true;
    core_thread.join();
}

void GB_Core::tick() {
    if(!paused()) return;

    //check the "mutex"
    while(loop_paused);

    cpu->clock_pulse();
}

void GB_Core::loop() {
    while(!thread_killed) {
        // a "mutex" designed to make sure I don't execute the clock_pulse function from 2 different threads
        // ie. the GB_Core::tick()
        loop_paused = true;
        while(thread_paused) if(thread_killed) return;
        loop_paused = false;

        auto start = std::chrono::high_resolution_clock::now();

        //every tick should take exactly 238 nanoseconds
        cpu->clock_pulse();

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::this_thread::sleep_for(std::chrono::nanoseconds(238)-duration);
    }
}

CPU::registers_t GB_Core::getRegistersFromCPU() {
    CPU::registers_t r = cpu->getRegisters();
    return r;
}