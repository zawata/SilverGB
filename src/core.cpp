#include "core.hpp"

#include <thread>
#include <condition_variable>
#include <chrono>

#include "defs.hpp"

#include "video.hpp"

static std::atomic<bool> pause_loop = ATOMIC_VAR_INIT(true);
static std::atomic<bool> loop_paused = ATOMIC_VAR_INIT(true);
static std::atomic<bool> kill_thread = ATOMIC_VAR_INIT(true);
static std::condition_variable cond;
static std::mutex m_lock;

GB_Core::GB_Core(File_Interface *rom, Configuration *cfg):
cart(new Cartridge(rom)),
io(new IO_Bus(cart, cfg, false)),
cpu(new CPU(io, cfg)) {
    screen_buffer = (u8 *)malloc(GB_S_P_SZ);

    vpu = new Video_Controller(io, cfg, screen_buffer);
}

GB_Core::~GB_Core() {
    //we didn't create vpu so we don't need to delete it.
    delete cpu;
    delete io;
    delete cart;
}

/**
 * Thread Functions
 */
void GB_Core::init_thread(bool paused) {
    kill_thread = false;
    pause_loop = paused;
    core_thread = std::thread(&GB_Core::run_thread, this);
}

void GB_Core::pause_thread() {
    pause_loop = true;
    while(!loop_paused);
}

bool GB_Core::thread_paused() {
    return loop_paused;
}

void GB_Core::resume_thread() {
    pause_loop = false;

    std::lock_guard<std::mutex> guard(m_lock);
    cond.notify_one();
}

void GB_Core::stop_thread() {
    kill_thread = true;
    if(thread_paused()) resume_thread();
    core_thread.join();
}

void GB_Core::run_thread() {
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

/**
 * Tick Functions
 */
void GB_Core::tick_once() {
    //can't check breakpoints on single tick functions
    cpu->tick();
    vpu->tick();
}

void GB_Core::tick_instr() {
    while(!cpu->tick()) {
        vpu->tick();
    }
    if(cpu->getRegisters().PC == breakpoint and bp_active)
        throw breakpoint_exception();
}

void GB_Core::tick_frame() {
    do {
        if(cpu->tick() && cpu->getRegisters().PC == breakpoint) {
            throw breakpoint_exception();
        }
    } while(!vpu->tick());
}

/**
 * Util Functions
 */
CPU::registers_t GB_Core::getRegistersFromCPU() {
    return cpu->getRegisters();
}

IO_Bus::io_registers_t GB_Core::getregistersfromIO() {
    return io->registers;
}

u8 GB_Core::getByteFromIO(u16 addr) {
    return io->read(addr);
}

void GB_Core::set_breakpoint(u16 bp) {
    breakpoint = bp;
    bp_active = true;
}

u8 *GB_Core::getScreenBuffer() {
    return screen_buffer;
}