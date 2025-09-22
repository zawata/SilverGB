#include "core.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <vector>

#include "util/bit.hpp"
#include "util/log.hpp"
#include "util/types/pixel.hpp"

#include "defs.hpp"
#include "joy.hpp"
#include "ppu.hpp"

using namespace jnk0le;

namespace Silver {
    Core::Core(
            const std::shared_ptr<Silver::File> &rom, const std::optional<std::shared_ptr<Silver::File>> &bootrom,
            gb_device_t device) :
        device(device) {
        // the Audio buffering system is threaded because it's simpler
        // luckily we have a single audio producer(main thread) and a single consumer(audio thread)
        // so we use an SPSC queue to buffer entire audio buffers at a time
        audio_queue  = new Ringbuffer<std::array<float, 2048>, 4>();
        audio_vector = std::vector<float>();
        audio_vector.reserve(2048);

        LogInfo("Core") << "Starting Core with CPU: " << cpu_names[device];

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

        // TOOD: switch to smart pointers
        cart = new Cartridge(rom);
        mem  = new Memory(device, bootrom.has_value());

        apu  = new APU(bootrom.has_value());
        ppu  = new PPU(cart, mem, device, bootrom.has_value());
        joy  = new Joypad(mem);

        io   = new IO_Bus(mem, apu, ppu, joy, cart, device, bootrom);
        cpu  = new CPU(mem, io, device, bootrom.has_value());
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

        if(cpu->getRegisters().PC == breakpoint && bp_active) {
            bp_active = false;
            throw breakpoint_exception();
        }
    }

    void Core::tick_frame() {
        u8   tick_cntr = 0;
        bool instr_completed;

        do {
            instr_completed = cpu->tick();
            apu->tick();
            this->frame_ready = ppu->tick();

            if(bp_active && instr_completed && cpu->getRegisters().PC == breakpoint) {
                bp_active = false;
                throw breakpoint_exception();
            }

            // TODO: this should be dynamic. probably adjusting the live sampling rate to keep the buffer from over or
            // underflowing.
            //  90 tends to underflow every couple frames, while 85 keeps buffer sizes at 2-3x higher than the callback
            //  copy size. the ideal rate right now seems to be 87, it will underflow every couple seconds
            if(tick_cntr++ == 87) {
                float left, right;
                apu->sample(&left, &right);
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

        u64                      nsDelta = (Clock::now() - last_invocation).count();
        u64                      totalTicks;

        if(dev_is_SGB(this->device)) {
            // 4.194304 MHz = 238.42 ns(p);
            totalTicks = nsDelta * 238.42f;
        } else if(dev_is_GBC(this->device)) {
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

    bool Core::is_frame_ready() { return this->frame_ready; }

    /**
     * Interface Functions
     */
    void Core::set_input_state(Joypad::button_states_t const &state) { joy->set_input_state(state); }

    void Core::do_audio_callback(float *buff, int copy_cnt) {
        if(audio_queue->isEmpty()) {
            LogWarn("Core") << "audio buffer underflow";
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

    const std::vector<Silver::Pixel> &Core::getPixelBuffer() { return this->ppu->getPixelBuffer(); }

    /**
     * Util Functions
     */
    void                              write_5bit_color(u8 *loc, u16 color) {
        *loc++ = (((color >> 0) & 0x001F) * 527 + 23) >> 6;
        *loc++ = (((color >> 5) & 0x001F) * 527 + 23) >> 6;
        *loc++ = (((color >> 10) & 0x001F) * 527 + 23) >> 6;
    }

    CPU::registers_t       Core::getRegistersFromCPU() { return cpu->getRegisters(); }

    Memory::io_registers_t Core::getregistersfromIO() { return mem->registers; }

    std::vector<u8>        Core::getOAMEntry(int index) {
        if(index >= 40) {
            return {};
        }

        PPU::obj_sprite_t sprite    = ppu->oam_fetch_sprite(index);

        u16               base_addr = 0x8000 & ((u16)(sprite.tile_num)) << 5;

        std::vector<u8>   ret_vec;
        u8                palette = mem->read_reg(Bit::test(sprite.attrs, 4) ? OBP1_REG : OBP0_REG);
        for(int i = 0; i < 16; i += 2) {
            u8 b1 = mem->read_oam(base_addr | i), b2 = mem->read_oam(base_addr | i + 1);

            for(int j = 0; j < 8; j++) {
                u8 out_color = ((b1 >> (7 - j)) & 1);
                out_color |= ((b2 >> (7 - j)) & 1) << 1;

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

    u16  Core::get_bp() { return breakpoint; }

    void Core::set_bp_active(bool en) { bp_active = en; }
    bool Core::get_bp_active() { return bp_active; }

#define Y_FLIP_BIT         6
#define X_FLIP_BIT         5
#define GBC_VRAM_BANK_BIT  3
#define GBC_PALETTE_MASK   0x7

#define BG_PRIORITY(attr)  (Bit::test((attr), PRIORITY_BIT))
#define BG_Y_FLIP(attr)    (Bit::test((attr), Y_FLIP_BIT))
#define BG_X_FLIP(attr)    (Bit::test((attr), X_FLIP_BIT))
#define BG_VRAM_BANK(attr) (Bit::test((attr), GBC_VRAM_BANK_BIT)) // false if bank 0
#define BG_PALETTE(attr)   ((attr) & GBC_PALETTE_MASK)
#define reg(X)             (mem->registers.X)

    std::pair<u16, u8> Core::calcTileAddrForCoordinate(bool window, u8 x_tile, u8 y) {
        u8   y_tile   = y >> 3;

        bool map_bit  = window ? Bit::test(reg(LCDC), 6) : Bit::test(reg(LCDC), 3);

        u16  base     = Bit::test(reg(LCDC), 3) ? 0x9C00 : 0x9800;
        u16  loc      = base + x_tile + (y_tile * 32);
        u8   tile_idx = mem->read_vram(loc, true, false);
        u8   bg_attr;

        if(dev_is_GBC(device) && !mem->get_dmg_compat_mode()) {
            bg_attr = mem->read_vram(loc, true, true);
        }

        u16 tile_addr = 0x8000;
        if(Bit::test(tile_idx, 7)) {
            tile_addr += 0x0800;
        } else if(!Bit::test(reg(LCDC), 4)) {
            tile_addr += 0x1000;
        }

        u8 tile_y_line = y & 0x7;
        if(dev_is_GBC(device) && !mem->get_dmg_compat_mode() && BG_Y_FLIP(bg_attr)) {
            tile_y_line = 7 - tile_y_line;
        }

        tile_addr += ((tile_idx & 0x7F) << 4) | (tile_y_line << 1);
        return {tile_addr, bg_attr};
    }

    std::pair<u8, u8> Core::getTileLineByAddr(u16 addr, bool bank1) {
        return {mem->read_vram(addr, true, bank1), mem->read_vram(addr + 1, true, bank1)};
    }

    // 16x8 tiles, 128x64 pixels
    void Core::getVRAMBuffer(std::vector<Pixel> &vec, u8 vramIdx, bool vramBank) {
        u16 baseAddr = 0x8000;

        if(vramIdx == 1) {
            baseAddr += 0x800;
        } else if(vramIdx == 2) {
            baseAddr += 0x1000;
        }

        for(u8 y = 0; y < 64; y++) {
            for(u8 x_tile = 0; x_tile < 16; x_tile++) {
                u8  y_tile      = y >> 3;
                u8  tile_idx    = (y_tile << 4) + x_tile;
                u8  tile_y_line = y & 0x7;
                u16 addr        = baseAddr;
                addr += ((tile_idx & 0x7F) << 4) | (tile_y_line << 1);

                u8 byte_1, byte_2;
                std::tie(byte_1, byte_2)             = getTileLineByAddr(addr, vramBank);

                std::array<PPU::fifo_color_t, 8> arr = {0};
                ppu->process_tile_line(arr, byte_1, byte_2, 0);
                for(const auto &color : arr) {
                    auto pixel = Silver::Pixel::makeFromRGB15(color.palette->colors[color.color_idx]);
                    vec.push_back(pixel);
                }
            }
        }
    }

    // 32 x 32 tiles, 256x256 pixels
    void Core::getBGBuffer(std::vector<Pixel> &vec) {
        for(int y = 0; y < 256; y++) {
            for(int x_tile = 0; x_tile < 32; x_tile++) {
                u16 tile_addr;
                u8  bg_attr;
                std::tie(tile_addr, bg_attr) = calcTileAddrForCoordinate(false, x_tile, y);

                u8 byte_1, byte_2;
                std::tie(byte_1, byte_2)             = getTileLineByAddr(tile_addr, BG_VRAM_BANK(bg_attr));

                std::array<PPU::fifo_color_t, 8> arr = {0};
                ppu->process_tile_line(arr, byte_1, byte_2, bg_attr);
                for(const auto &color : arr) {
                    auto pixel = Silver::Pixel::makeFromRGB15(color.palette->colors[color.color_idx]);
                    vec.push_back(pixel);
                }
            }
        }
    }

    // void Core::getWNDBuffer(std::vector<Pixel> &vec) {
    //     for(int y = 0; y < 256; y++) {
    //         for(int x_tile = 0; x_tile < 32; x_tile++) {
    //             u16 tile_addr;
    //             u8  bg_attr;
    //             std::tie(tile_addr, bg_attr) = calcTileAddrForCoordinate(false, x_tile, y);
    //
    //             u8 byte_1, byte_2;
    //             std::tie(byte_1, byte_2)             = getTileLineByAddr(tile_addr, BG_VRAM_BANK(bg_attr));
    //
    //             std::array<PPU::fifo_color_t, 8> arr = {0};
    //             ppu->process_tile_line(arr, byte_1, byte_2, bg_attr);
    //             for(const auto &color : arr) {
    //                 auto pixel = Silver::Pixel::makeFromRGB15(color.palette->colors[color.color_idx]);
    //                 vec.push_back(pixel);
    //             }
    //         }
    //     }
    // }

#undef reg
} // namespace Silver
