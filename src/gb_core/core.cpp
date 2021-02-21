#include <chrono>

#include "gb_core/core.hpp"
#include "gb_core/defs.hpp"
#include "gb_core/input.hpp"
#include "gb_core/io_reg.hpp"
#include "gb_core/ppu.hpp"
#include "util/bit.hpp"

GB_Core::GB_Core(Silver::File*rom, Silver::File *bootrom = nullptr) {
    screen_buffer = (u8 *)malloc(GB_S_P_SZ);

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

    free(screen_buffer);
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
    do {
        if(cpu->tick() && cpu->getRegisters().PC == breakpoint && bp_active) {
            bp_active = false;
            throw breakpoint_exception();
        }
        apu->tick();
    } while(!vpu->tick());
}

void GB_Core::tick_audio_buffer(u8* buf, int buf_len) {
    int rem_buf_sz = buf_len,
        tick_cntr = 0;

    while(rem_buf_sz > 0) {
        tick_cntr++;
        cpu->tick();
        vpu->tick();
        apu->tick();

        if (tick_cntr++ == 87) { //TODO: demagic
            *buf++ = apu->sample();
            tick_cntr = 0;
        }
    }
}

/**
 * Interface Functions
 */
void GB_Core::set_input_state(Input_Manager::button_states_t const& state) {
    io->input->set_input_state(state);
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

    //int x = 0, y = 0, p = 0;
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
    return 0;
}

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

//void GB_Core::__fetch_tile(u16 tile_addr, u8 x, u16 &tile_byte_1, u16 tile_byte_2) {
//    
//}

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