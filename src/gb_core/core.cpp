#include <chrono>

#include "gb_core/core.hpp"
#include "gb_core/defs.hpp"
#include "gb_core/input.hpp"
#include "gb_core/video.hpp"
#include "util/bit.hpp"

GB_Core::GB_Core(Silver::File*rom, Silver::File *bootrom = nullptr) {
    screen_buffer = (u8 *)malloc(GB_S_P_SZ);

    cart = new Cartridge(rom);
    io = new IO_Bus(cart, false, bootrom);
    cpu = new CPU(io, bootrom != nullptr);
    vpu = new Video_Controller(io, screen_buffer, bootrom != nullptr);
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
}

void GB_Core::tick_instr() {
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
    return 0;
}

u8 const* GB_Core::getScreenBuffer() {
    return screen_buffer;
}

std::vector<u8> GB_Core::getOAMEntry(int index) {
    if(index >= 40 ) { return {}; }

    Video_Controller::obj_sprite_t sprite = vpu->oam_fetch_sprite(index);

    u16 base_addr = 0x8000 & ((u16)(sprite.tile_num)) << 5;

    std::vector<u8> ret_vec;
    u8 pallette = io->read_reg( Bit.test(sprite.attrs, 4) ? OBP1_REG : OBP0_REG);
    for( int i = 0; i < 16; i += 2 ) {
        u8 b1 = io->read_oam(base_addr | i),
           b2 = io->read_oam(base_addr | i + 1);

        for(int i = 0; i < 8; i++) {
            u8 out_color = ((b1 >> (7 - i)) & 1);
            out_color   |= ((b2 >> (7 - i)) & 1) << 1;

            const u8 *pixel_colors = Video_Controller::pixel_colors[(pallette >> (out_color * 2)) & 0x3];
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