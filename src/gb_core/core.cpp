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
    #pragma unroll
    for(int i = 0; i < 4; i++) {
        cpu->tick();
        vpu->tick();
    }

    if(cpu->getRegisters().PC == breakpoint && bp_active){
        bp_active = false;
        throw breakpoint_exception();
    }
}

void GB_Core::tick_frame() {
    do {
        if(cpu->tick() && cpu->getRegisters().PC == breakpoint && bp_active) {
            bp_active = false;
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
    std::ofstream output_file("test.ppm");

    output_file << "P3 160 144 255\n" << std::flush;

    int x = 0, y = 0, p = 0;
    for(int i = 0; i < (160 * 144 * 3); i++) {
        // p = i%3;
        // y = (i/3)/160;
        // x = (i/3)%160;

        // if(!p) {
        //     if(!x && y%8 == 0) {
        //         for(int j = 0; j < 180; j++)
        //             output_file << "255 0 0 ";
        //     }

        //     if(x%8 == 0) {
        //         output_file << "255 0 0 ";
        //     }
        // }

        output_file << (int)screen_buffer[i] << " ";
    }
    output_file.close();
}

void GB_Core::set_bp(u16 bp, bool en) {
    breakpoint = bp;
    set_bp_active(en);
}

u16 GB_Core::get_bp() { return breakpoint; }

void GB_Core::set_bp_active(bool en) { bp_active = en; }
bool GB_Core::get_bp_active()        { return bp_active; }

u8 *GB_Core::getScreenBuffer() {
    return screen_buffer;
}

// u8 *GB_Core::getVRAMBuffer() {
//     u8 *map = (u8 *)malloc(64*128);
//     for(int y = 0; y < 64; y++) {
//         for(int x = 0; x < 128; x++) {
//             u8 tile_start_x = x / 8,
//                tile_start_y = y / 8,
//                tile_x =       x % 8,
//                tile_y =       y % 8;

//             u16 tile_addr = (tile_start_x*0x10) + (tile_start_y * 0x100);

//             // std::cout << "x,y: " << x << " " << y << std::endl;
//             // std::cout << "tile_: " << as_hex(tile_addr) << std::endl;
//             // std::cout << "tile_byte_: " << as_hex(tile_addr + (tile_y*2)) << std::endl;
//             // std::cout << "tile_bit_: " << as_hex(tile_x) << std::endl;

//             u8 byte_1 = io->video_ram_char[tile_addr + (tile_y*2)],
//                byte_2 = io->video_ram_char[tile_addr + (tile_y*2) + 1];

//             //std::cout << "tile_data: " << as_hex(byte_1) << " " << as_hex(byte_2) << std::endl;
//             //std::cout << "tile_out: " << as_hex(((byte_1 >> (tile_x-1))) | (byte_2 >> tile_x)) << std::endl;
//             map[(y*128) + x] = ((byte_1 >> (7 - tile_x)) & 1) << 1;
//             map[(y*128) + x] |= ((byte_2 >> (7 - tile_x)) & 1);
//         }
//     }

//     for(int y = 0; y < 64; y++) {
//         for(int x = 0; x < 128; x++) {
//             u8 byte = map[(y*128) + x];
//             //std::cout << "x,y: d " << x << " " << y << ": " << as_hex(byte) << std::endl;

//             switch(byte) {
//             case 0:
//                 output_file << "255 255 255 ";
//                 break;
//             case 1:
//                 output_file << "0 0 0 ";
//                 break;
//             case 2:
//                 output_file << "0 0 0 ";
//                 break;
//             case 3:
//                 output_file << "0 0 0 ";
//                 break;
//             }
//         }
//     }
// }