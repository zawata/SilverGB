#include "core.hpp"

#include <thread>
#include <condition_variable>
#include <chrono>

static std::atomic<bool> pause_loop = ATOMIC_VAR_INIT(true);
static std::atomic<bool> loop_paused = ATOMIC_VAR_INIT(true);
static std::atomic<bool> kill_thread = ATOMIC_VAR_INIT(true);
static std::condition_variable cond;
static std::mutex m_lock;

GB_Core::GB_Core(File_Interface *rom, Configuration *cfg):
cart(new Cartridge(rom)),
io(new IO_Bus(cart, cfg, false)),
cpu(new CPU(io, cfg)),
//pointer only valid for lifetime of io.
//does not need to be deleted.
vpu(io->vpu) {}

GB_Core::~GB_Core() {
    //we didn't create vpu so we don't need to delete it.
    delete cpu;
    delete io;
    delete cart;
}

void GB_Core::start(bool paused) {
    kill_thread = false;
    pause_loop = paused;
    core_thread = std::thread(&GB_Core::loop, this);
}

void GB_Core::pause() {
    pause_loop = true;
    while(!loop_paused);
}

bool GB_Core::paused() {
    return loop_paused;
}

void GB_Core::resume() {
    pause_loop = false;

    std::lock_guard<std::mutex> guard(m_lock);
    cond.notify_one();
}

void GB_Core::stop() {
    kill_thread = true;
    if(paused()) resume();
    core_thread.join();
}

// void GB_Core::next_instruction() {
//     //we can only work if the thread is paused
//     if(!paused()) return;

//     while(!cpu->tick());
// }


void GB_Core::loop() {
    while(!kill_thread) {
        if(pause_loop) {
            loop_paused = true;
            std::unique_lock<std::mutex> l(m_lock);
            cond.wait(l);
            loop_paused = false;
        }

        int v_step;

        auto start = std::chrono::high_resolution_clock::now();
        do {
            cpu->tick();
        } while(!vpu->tick());
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        std::cout << duration.count() << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(16666)-duration);
    }
}



CPU::registers_t GB_Core::getRegistersFromCPU() {
    CPU::registers_t r = cpu->getRegisters();
    return r;
}

u8 GB_Core::getByteFromIO(u16 addr) {
    return io->read(addr);
}