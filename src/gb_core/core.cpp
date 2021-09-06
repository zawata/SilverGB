#include <chrono>

#include "gb_core/core.hpp"
#include "gb_core/defs.hpp"
#include "gb_core/input.hpp"
#include "gb_core/io_reg.hpp"
#include "gb_core/ppu.hpp"
#include "util/bit.hpp"
#include "util/util.hpp"

GB_Core::GB_Core(Silver::File*rom, Silver::File *bootrom = nullptr) {
    screen_buffer = new u8[GB_S_P_SZ];

    // the Aduio buffering system is threaded because it's simpler
    // luckily we have a single audio producer(main thread) and a single consumer(audio thread)
    // so we use an SPSC queue to buffer entire audio buffers at a time
    audio_queue = new rigtorp::SPSCQueue<std::vector<float>>(4);
    audio_vector = std::vector<float>();
    audio_vector.reserve(2048);

    cart = new Cartridge(rom);
    apu = new APU(bootrom != nullptr);

    io = new IO_Bus(apu, cart, false, bootrom);
    cpu = new CPU(io, bootrom != nullptr);
    vpu = new PPU(io, screen_buffer, bootrom != nullptr);
}

GB_Core::~GB_Core() {
    delete vpu;
    delete cpu;
    delete io;
    delete cart;

    delete[] screen_buffer;
}

/**
 * Tick Functions
 */
void GB_Core::tick_once() {
    //can't check breakpoints on single tick functions
    cpu->tick();
    vpu->tick();
    apu->tick();
}

void GB_Core::tick_instr() {
    for(int i = 0; i < 4; i++) {
        cpu->tick();
        vpu->tick();
        apu->tick();
    }

    if(cpu->getRegisters().PC == breakpoint && bp_active){
        bp_active = false;
        throw breakpoint_exception();
    }
}

void GB_Core::tick_frame() {
    u8 tick_cntr = 0;

    do {
        if(cpu->tick() && cpu->getRegisters().PC == breakpoint && bp_active) {
            bp_active = false;
            throw breakpoint_exception();
        }

        apu->tick();
        //TODO: this should be dynamic. probably adjusting the live sampling rate ot keep the buffer from over or underflowing.
        // 90 tends to underflow every couple frames, while 85 keeps buffer sizes at 2-3x higher than the callback copy size.
        // the ideal rate right now seems to be 87, it will underflow every couple seconds
        if(tick_cntr++ == 87) {
            float left,right;
            apu->sample(&left,&right);
            audio_vector.push_back(left);
            tick_cntr = 0;
        }

        if(audio_vector.size() == 2048) { //TODO: make constant
            audio_queue->try_push(audio_vector);
            audio_vector.clear();
        }

    } while(!vpu->tick());
}

/**
 * Interface Functions
 */
void GB_Core::set_input_state(Input_Manager::button_states_t const& state) {
    io->input->set_input_state(state);
}

void GB_Core::do_audio_callback(float *buff, int copy_cnt) {
    if(!audio_queue->front()) {
        std::cerr << "audio buffer underflow" << std::endl;
        memset(buff, 0, copy_cnt * 4);
    } else {
        auto audio_buffer = *audio_queue->front();
        audio_queue->pop();

        assert(copy_cnt == audio_buffer.size());

        int type_sz = sizeof(decltype(audio_buffer)::value_type);

        memcpy(buff, audio_buffer.data(), audio_buffer.size() * type_sz);
    }
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

u8 GB_Core::getByteFromIO(u16 addr) { return 0;  }

u8 const* GB_Core::getScreenBuffer() {
    return screen_buffer;
}

std::vector<u8> GB_Core::getOAMEntry(int index) {
    if(index >= 40 ) { return {}; }

    PPU::obj_sprite_t sprite = vpu->oam_fetch_sprite(index);

    u16 base_addr = 0x8000 & ((u16)(sprite.tile_num)) << 5;

    std::vector<u8> ret_vec;
    u8 pallette = io->read_reg( Bit::test(sprite.attrs, 4) ? OBP1_REG : OBP0_REG);
    for( int i = 0; i < 16; i += 2 ) {
        u8 b1 = io->read_oam(base_addr | i),
           b2 = io->read_oam(base_addr | i + 1);

        for(int i = 0; i < 8; i++) {
            u8 out_color = ((b1 >> (7 - i)) & 1);
            out_color   |= ((b2 >> (7 - i)) & 1) << 1;

            const u8 *pixel_colors = PPU::pixel_colors[(pallette >> (out_color * 2)) & 0x3];
            ret_vec.push_back(pixel_colors[0]);
            ret_vec.push_back(pixel_colors[1]);
            ret_vec.push_back(pixel_colors[2]);
        }
    }

    return ret_vec;
}

/**
 * Breakpoint Functions
 */
void GB_Core::set_bp(u16 bp, bool en) {
    breakpoint = bp;
    set_bp_active(en);
}

u16 GB_Core::get_bp() { return breakpoint; }

void GB_Core::set_bp_active(bool en) { bp_active = en; }
bool GB_Core::get_bp_active()        { return bp_active; }


void GB_Core::getBGBuffer(u8 *buf) {
    #define reg(X) (io->registers.X)

    for(int y = 0; y < 256; y++) {
        for(int x = 0; x < 32; x++) {

            u8 y_tile = y / 8;

            u16 base = Bit::test(reg(LCDC), 3) ? 0x9C00 : 0x9800;
            u8 bg_idx = io->read_vram(base + x + (y_tile * 32));

            u16 tile_addr = 0;
            if(Bit::test(bg_idx, 7)) {
                tile_addr = 0x0800;
            }
            else if(!Bit::test(reg(LCDC), 4)) {
                tile_addr = 0x1000;
            }

            tile_addr +=
                    ((bg_idx & 0x7F) << 4) |
                    ((y & 0x7) << 1);

            u8 byte_1 = io->read_vram(0x8000 + tile_addr, true),
               byte_2 = io->read_vram(0x8000 + tile_addr + 1, true);

            for(int tile_x = 0; tile_x < 8; tile_x++) {
                u8 tile_idx = ((byte_1 >> (7 - tile_x)) & 1);
                tile_idx   |= ((byte_2 >> (7 - tile_x)) & 1) << 1;
                tile_idx *= 2;

                memcpy(&buf[((y*256) + (x * 8) + tile_x) * 3], PPU::pixel_colors[(reg(BGP) >> tile_idx) & 0x3], 3);
            }
        }
    }
}

void GB_Core::getWNDBuffer(u8 *buf) {
    #define reg(X) (io->registers.X)

    for(int y = 0; y < 256; y++) {
        for(int x = 0; x < 32; x++) {

            u8 y_tile = y / 8;

            u16 base = Bit::test(reg(LCDC), 6) ? 0x9C00 : 0x9800;
            u8 bg_idx = io->read_vram(base + x + (y_tile * 32));

            u16 tile_addr = 0;
            if(Bit::test(bg_idx, 7)) {
                tile_addr = 0x0800;
            }
            else if(!Bit::test(reg(LCDC), 4)) {
                tile_addr = 0x1000;
            }

            tile_addr +=
                    ((bg_idx & 0x7F) << 4) |
                    ((y & 0x7) << 1);

            u8 byte_1 = io->read_vram(0x8000 + tile_addr, true),
               byte_2 = io->read_vram(0x8000 + tile_addr + 1, true);

            for(int tile_x = 0; tile_x < 8; tile_x++) {
                u8 tile_idx = ((byte_1 >> (7 - tile_x)) & 1);
                tile_idx   |= ((byte_2 >> (7 - tile_x)) & 1) << 1;
                tile_idx *= 2;

                memcpy(&buf[((y*256) + (x * 8) + tile_x) * 3], PPU::pixel_colors[(reg(BGP) >> tile_idx) & 0x3], 3);
            }
        }
    }
}