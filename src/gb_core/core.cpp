#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <ratio>
#include <vector>

#include <nowide/iostream.hpp>

#include "core.hpp"
#include "defs.hpp"
#include "joy.hpp"
#include "util/types/pixel.hpp"
#include "ppu.hpp"
#include "util/bit.hpp"
#include "util/util.hpp"

using namespace jnk0le;

namespace Silver {

Core::Core(Silver::File *rom, Silver::File *bootrom, gb_device_t device):
device(device) {
    // the Audio buffering system is threaded because it's simpler
    // luckily we have a single audio producer(main thread) and a single consumer(audio thread)
    // so we use an SPSC queue to buffer entire audio buffers at a time
    audio_queue = new Ringbuffer<std::array<float, 2048>, 4>();
    audio_vector = std::vector<float>();
    audio_vector.reserve(2048);

    nowide::cout << "Starting Core with CPU: " << cpu_names[device] << std::endl;

    // Class Usage Heirarchy in diagram form
    // +-----+    +-----+      +------+
    // | CPU | -> | IO_ | -+-> | CART |
    // +-----+    +-----+  |   +------+
    //    |          |     |
    //    |          |     |   +-----+
    //    |          |     +-> | PPU | --+
    //    |          |     |   +-----+   |
    //    |          |     |             |
    //    |          |     |   +-----+   |
    //    |          |     +-> | APU | --+
    //    |          |     |   +-----+   |
    //    |          |     |             |
    //    |          |     |   +-----+   |
    //    |          |     +-> | JOY | --+
    //    |          |         +-----+   |
    //    |          |                   |
    //    |          |                   |   +-----+
    //    +----------+-------------------+-> | MEM |
    //                                       +-----+

    cart = new Cartridge(rom);
    mem = new Memory(device, bootrom != nullptr);

    apu = new APU(bootrom != nullptr);
    ppu = new PPU(cart, mem, device, bootrom != nullptr);
    joy = new Joypad(mem);

    io = new IO_Bus(mem, apu, ppu, joy, cart, device, bootrom);
    cpu = new CPU(mem, io, device, bootrom != nullptr);
}

Core::~Core() {
    delete cpu;
    delete io;
    delete joy;
    delete ppu;
    delete apu;
    delete mem;
    delete cart;
    delete audio_queue;
}

/**
 * Tick Functions
 */
void Core::tick_once() {
    // can't check breakpoints on single tick functions
    cpu->tick();
    apu->tick();
    this->frame_ready = ppu->tick();
}

void Core::tick_instr() {
    for(int i = 0; i < 4; i++) {
        tick_once();
    }

    if(cpu->getRegisters().PC == breakpoint && bp_active){
        bp_active = false;
        throw breakpoint_exception();
    }
}

void Core::tick_frame() {
    u8 tick_cntr = 0;
    bool instr_completed;

    do {
        instr_completed = cpu->tick();
        apu->tick();
        this->frame_ready = ppu->tick();


        if(bp_active && instr_completed && cpu->getRegisters().PC == breakpoint) {
            bp_active = false;
            throw breakpoint_exception();
        }

        //TODO: this should be dynamic. probably adjusting the live sampling rate to keep the buffer from over or underflowing.
        // 90 tends to underflow every couple frames, while 85 keeps buffer sizes at 2-3x higher than the callback copy size.
        // the ideal rate right now seems to be 87, it will underflow every couple seconds
        if(tick_cntr++ == 87) {
            float left,right;
            apu->sample(&left,&right);
            audio_vector.push_back(left);
            tick_cntr = 0;
        }

        if(audio_vector.size() == audio_buffer_sz) {
            AudioBuffer buf;
            std::copy(audio_vector.begin(), audio_vector.end(), buf.begin());
            audio_vector.clear();

            audio_queue->insert(buf);
        }

    } while(!this->frame_ready);
}

// TODO: check this implementation later
void Core::tick_delta_or_frame() {
    using Clock = std::chrono::high_resolution_clock;
    static Clock::time_point last_invocation;

    u64 nsDelta = (Clock::now() - last_invocation).count();
    u64 totalTicks;

    if (dev_is_SGB(this->device)) {
        // 4.194304 MHz = 238.42 ns(p);
        totalTicks = nsDelta * 238.42f;
    } else if (dev_is_GBC(this->device)) {
        // 8.388688 MHz = 119.21 ns(p);
        totalTicks = nsDelta * 119.21f;
    } else {
        // 4.295454 MHz = 232.80 ns(p);
        totalTicks = nsDelta * 232.8f;
    }

    for(u64 i; i < totalTicks || !this->frame_ready; i++) {
        this->tick_once();
    }

    last_invocation = Clock::now();
}

bool Core::is_frame_ready() {
  return this->frame_ready;
}

/**
 * Interface Functions
 */
void Core::set_input_state(Joypad::button_states_t const& state) {
    joy->set_input_state(state);
}

void Core::do_audio_callback(float *buff, int copy_cnt) {
    if(audio_queue->isEmpty()) {
        nowide::cerr << "audio buffer underflow" << std::endl;
        memset(buff, 0, copy_cnt * 4);
    } else {
        // TODO: don't want to do 2 copies of this data
        AudioBuffer audio_buffer;
        audio_queue->remove(audio_buffer);

        assert(copy_cnt == audio_buffer.size());

        int type_sz = sizeof(decltype(audio_buffer)::value_type);
        memcpy(buff, audio_buffer.data(), audio_buffer.size() * type_sz);
    }
}

const std::vector<Silver::Pixel> &Core::getPixelBuffer() {
    return this->ppu->getPixelBuffer();
}

/**
 * Util Functions
 */
void write_5bit_color(u8 *loc, u16 color) {
    *loc++ = (((color >> 0)  & 0x001F) * 527 + 23 ) >> 6;
    *loc++ = (((color >> 5)  & 0x001F) * 527 + 23 ) >> 6;
    *loc++ = (((color >> 10) & 0x001F) * 527 + 23 ) >> 6;
}

CPU::registers_t Core::getRegistersFromCPU() {
    return cpu->getRegisters();
}

Memory::io_registers_t Core::getregistersfromIO() {
    return mem->registers;
}

u8 Core::getByteFromIO(u16 addr) { return 0; }

std::vector<u8> Core::getOAMEntry(int index) {
    if(index >= 40 ) { return {}; }

    PPU::obj_sprite_t sprite = ppu->oam_fetch_sprite(index);

    u16 base_addr = 0x8000 & ((u16)(sprite.tile_num)) << 5;

    std::vector<u8> ret_vec;
    u8 palette = mem->read_reg( Bit::test(sprite.attrs, 4) ? OBP1_REG : OBP0_REG);
    for( int i = 0; i < 16; i += 2 ) {
        u8 b1 = mem->read_oam(base_addr | i),
           b2 = mem->read_oam(base_addr | i + 1);

        for(int j = 0; j < 8; j++) {
            u8 out_color = ((b1 >> (7 - j)) & 1);
            out_color   |= ((b2 >> (7 - j)) & 1) << 1;

            u8 colors[3] = {0};
            write_5bit_color(colors, PPU::gb_palette.colors[(palette >> (out_color * 2)) & 0x3]);
            ret_vec.push_back(colors[0]);
            ret_vec.push_back(colors[1]);
            ret_vec.push_back(colors[2]);
        }
    }

    return ret_vec;
}

/**
 * Breakpoint Functions
 */
void Core::set_bp(u16 bp, bool en) {
    breakpoint = bp;
    set_bp_active(en);
}

u16 Core::get_bp() { return breakpoint; }

void Core::set_bp_active(bool en) { bp_active = en; }
bool Core::get_bp_active()        { return bp_active; }

#define Y_FLIP_BIT        6
#define X_FLIP_BIT        5
#define GBC_VRAM_BANK_BIT 3
#define GBC_PALETTE_MASK  0x7

#define BG_PRIORITY(attr)       (Bit::test((attr), PRIORITY_BIT))
#define BG_Y_FLIP(attr)         (Bit::test((attr), Y_FLIP_BIT))
#define BG_X_FLIP(attr)         (Bit::test((attr), X_FLIP_BIT))
#define BG_VRAM_BANK(attr)      (Bit::test((attr), GBC_VRAM_BANK_BIT)) //false if bank 0
#define BG_PALETTE(attr)        ((attr) & GBC_PALETTE_MASK)

//TODO: de-duplicate these functions

void Core::getBGBuffer(u8 *buf) {
    #define reg(X) (mem->registers.X)

    for(int y = 0; y < 256; y++) {
        for(int x = 0; x < 32; x++) {

            u8 y_tile = y / 8;

            u16 base = Bit::test(reg(LCDC), 3) ? 0x9C00 : 0x9800;
            u16 loc = base + x + (y_tile * 32);
            u8 bg_idx = mem->read_vram(loc, true, false);
            u8 bg_attr;

            if(dev_is_GBC(device)) {
                bg_attr = mem->read_vram(loc, true, true);
            }

            u16 tile_addr = 0;
            if(Bit::test(bg_idx, 7)) {
                tile_addr = 0x0800;
            }
            else if(!Bit::test(reg(LCDC), 4)) {
                tile_addr = 0x1000;
            }

            u8 tile_y_line = y & 0x7;
            if(dev_is_GBC(device) && BG_Y_FLIP(bg_attr)) tile_y_line = 7 - tile_y_line;

            tile_addr +=
                    ((bg_idx & 0x7F) << 4) |
                    (tile_y_line << 1);

            u8 byte_1 = mem->read_vram(0x8000 + tile_addr, true, BG_VRAM_BANK(bg_attr)),
               byte_2 = mem->read_vram(0x8000 + tile_addr + 1, true, BG_VRAM_BANK(bg_attr));

            for(int tile_x = 0; tile_x < 8; tile_x++) {
                u8 tile_x_bit = ((dev_is_GBC(device) && BG_X_FLIP(bg_attr)) ? (tile_x) : (7 - tile_x)) ;

                u8 tile_idx = ((byte_1 >> tile_x_bit) & 1);
                tile_idx   |= ((byte_2 >> tile_x_bit) & 1) << 1;

                u8 color_idx;
                u8 palette_idx;
                if(dev_is_GBC(device)) {
                    color_idx = tile_idx & 0x3_u8;
                    palette_idx = BG_PALETTE(bg_attr);
                } else {
                    tile_idx <<= 1;
                    color_idx = (reg(BGP) >> (tile_idx << 1)) & 0x3_u8;
                    palette_idx = 0;
                }

                auto p = Silver::Pixel::makeFromRGB15(ppu->bg_palettes[palette_idx].colors[color_idx]);
                PixelBufferEncoder<u8>::WritePixel<PixelFormat::RGB>::write(
                    &buf[((y*256) + (x * 8) + tile_x) * 3], 
                    p
                );
            }
        }
    }

    #undef reg
}

void Core::getWNDBuffer(u8 *buf) {
    #define reg(X) (mem->registers.X)

    for(int y = 0; y < 256; y++) {
        for(int x = 0; x < 32; x++) {

            u8 y_tile = y / 8;

            u16 base = Bit::test(reg(LCDC), 6) ? 0x9C00 : 0x9800;
            u16 loc = base + x + (y_tile * 32);
            u8 bg_idx = mem->read_vram(loc, true, false);
            u8 bg_attr;

            if(dev_is_GBC(device)) {
                bg_attr = mem->read_vram(loc, true, true);
            }

            u16 tile_addr = 0;
            if(Bit::test(bg_idx, 7)) {
                tile_addr = 0x0800;
            }
            else if(!Bit::test(reg(LCDC), 4)) {
                tile_addr = 0x1000;
            }

            u8 tile_y_line = y & 0x7;
            if(dev_is_GBC(device) && BG_Y_FLIP(bg_attr)) tile_y_line = 7 - tile_y_line;

            tile_addr +=
                    ((bg_idx & 0x7F) << 4) |
                    (tile_y_line << 1);

            u8 byte_1 = mem->read_vram(0x8000 + tile_addr, true, BG_VRAM_BANK(bg_attr)),
               byte_2 = mem->read_vram(0x8000 + tile_addr + 1, true, BG_VRAM_BANK(bg_attr));

            for(int tile_x = 0; tile_x < 8; tile_x++) {
                u8 tile_x_bit = ((dev_is_GBC(device) && BG_X_FLIP(bg_attr)) ? (tile_x) : (7 - tile_x)) ;

                u8 tile_idx = ((byte_1 >> tile_x_bit) & 1);
                tile_idx   |= ((byte_2 >> tile_x_bit) & 1) << 1;
                u8 color_idx;
                u8 palette_idx;
                if(dev_is_GBC(device)) {
                    color_idx = tile_idx & 0x3_u8;
                    palette_idx = BG_PALETTE(bg_attr);
                } else {
                    tile_idx <<= 1;
                    color_idx = (reg(BGP) >> (tile_idx << 1)) & 0x3_u8;
                    palette_idx = 0;
                }

                auto p = Silver::Pixel::makeFromRGB15(ppu->bg_palettes[palette_idx].colors[color_idx]);
                PixelBufferEncoder<u8>::WritePixel<PixelFormat::RGB>::write(
                    &buf[((y*256) + (x * 8) + tile_x) * 3], 
                    p
                );
            }
        }
    }
    #undef reg
}

} // namespace Silver
