#pragma once

#include <ratio>
#include <sstream>
#include <type_traits>
#include <vector>
#include <deque>

#include "cart.hpp"
#include "defs.hpp"
#include "mem.hpp"

#include "util/bit.hpp"
#include "util/types/primitives.hpp"
#include "util/types/circular_queue.hpp"
#include "util/types/pixel.hpp"
#include "util/util.hpp"

class PPU {
public:
    static constexpr u32 native_width = 160;
    static constexpr u32 native_height = 144;
    static constexpr u32 native_pixel_count = native_width * native_height;

    enum {
        SCANLINE_OAM  = 2,
        SCANLINE_VRAM = 3,
        HBLANK        = 0,
        VBLANK        = 1,
    } v_mode_t;

    typedef struct {
        u8 pos_y;
        u8 pos_x;
        u8 tile_num;
        u8 attrs;
        // u8 obj_idx;
    } obj_sprite_t;

    typedef struct palette_t {
        u16 colors[4];

        std::string to_rgb15_string() {
            return itoh(colors[0], 4, true) + " " + itoh(colors[1], 4, true) + " " + itoh(colors[2], 4, true) + " " + itoh(colors[3], 4, true);
        }

        std::string to_rgb24_string() {
            std::stringstream ss;
            ss << std::hex;
            for(int i = 0; i < 4; i++) {
                if(i != 0) ss << ", ";
                auto p = Silver::Pixel::makeFromRGB15(colors[i]);
                ss << "#" << itoh(p.r, 2, true, false) << itoh(p.g, 2, true, false) << itoh(p.b, 2, true, false);
            }
            return ss.str();
        }
    } palette_t;

    struct fifo_color_t {
        u8 color_idx;
        palette_t *palette;
        bool is_transparent;
        bool priority;
    };

    // TODO: make this user-configurable
    // http://www.budmelvin.com/dev/15bitconverter.html
    // static constexpr palette_t gb_palette = {
    //     0x7FFF, // white
    //     0x6B5A, // lightgrey
    //     0x35AD, // dimgrey
    //     0x0000, // black
    // };

    static constexpr palette_t gb_palette = {
        0x6BDC, // white
        0x3B10, // lightgrey
        0x29C6, // dimgrey
        0x1081, // black
    };

    PPU(
        Cartridge *cart,
        Memory *mem,
        gb_device_t device,
        bool bootrom_enabled
    );
    ~PPU();

    bool tick();
    obj_sprite_t oam_fetch_sprite(int index);
    void write_bg_color_data(u8 data);
    u8 read_bg_color_data();
    void write_obj_color_data(u8 data);
    u8 read_obj_color_data();

    void set_obj_priority(bool obj_has_priority);

    const std::vector<Silver::Pixel> &getPixelBuffer();

    //public because core needs access //TODO: befriend core maybe?
    palette_t bg_palettes[8];
    palette_t obj_palettes[8];

private:
    void enqueue_sprite_data(PPU::obj_sprite_t const& curr_sprite);

    void ppu_tick_oam();
    void ppu_tick_vram();

    bool isGBCAllowed();
    void set_color_data(u8 *reg, palette_t *palette_mem, u8 data);
    u8 get_color_data(u8 *reg, palette_t *palette_mem);

    Cartridge *cart;
    Memory *mem;

    gb_device_t device;
    std::vector<Silver::Pixel> pixBuf;

    int curr_mode;

    /**
     * PPU Variables
     */
    CircularQueue<fifo_color_t> *bg_fifo = new CircularQueue<fifo_color_t>(160);
    CircularQueue<fifo_color_t> *sp_fifo = new CircularQueue<fifo_color_t>(160);
    u8 process_step;

    int frame_clock_count = 0; // count of clocks in a frame
    int line_clock_count = 0;  // count of clocks in a line
    int pix_clock_count = 0; //pixels clocked in currenrt line

    enum __OAM_fetch_steps {
        OAM_0,
        OAM_1
    } oam_fetch_step;

    enum __VRAM_fetch_steps {
        BM_0, //Background Map clk 1
        BM_1, //Background Map clk 2

        WM_0, //Window Map clk 1
        WM_1, //Window Map clk 2;

        TD_0_0, //Tile Data byte 1 clk 1
        TD_0_1, //Tile Data byte 1 clk 2

        TD_1_0, //Tile Data byte 2 clk 1
        TD_1_1, //Tile Data byte 2 clk 2

        IDLE,  //Idle Clock
    };
    __VRAM_fetch_steps vram_fetch_step = IDLE;

    u8 old_LY  = 0;
    bool
        coin_bit_signal = false,
        pause_bg_fifo = true,
        skip_sprite_clock = false,
        obj_priority_mode = true;

    u8 old_LCDC;

    u16
        bg_map_addr,    // addr of the current tile in the bg  tile map
        wnd_map_addr,   // addr of the current tile in the wnd tile map
        tile_addr;      // relative addr of the current tile data to fetch

    u8
        attr_byte,      // GBC attributes of bg_map_byte and wnd_map_byte
        bg_map_byte,    // byte from the current tile in the bg tile map
        wnd_map_byte,   // byte from the current tile in the wnd tile map
        tile_byte_1,    // 1st byte from the current tile fetched
        tile_byte_2;    // 1st byte from the current tile fetched

    bool
        bg_map_bank_1;  // is the tile data for the bg tile data in bank 0 or 1;

    u8
        tile_y_line, // GBC tile y pixel counter
        wnd_y_cntr, // window y counter
        y_cntr, // y counter
        x_cntr; // x counter

    bool
        first_frame = false,   // first frame after lcd enable
        frame_disable = false, // disable pixel output for current frame(used with first frame)
        new_frame = false,     // the first clock tick of a new frame
        new_line = false,      // the first clock tick of a new line
        skip_fetch = false,    // the first VRAM access of a new line
        in_window = false;     // are we in window mode

    int sprite_counter = 0;
    obj_sprite_t current_sprite;
    std::vector<obj_sprite_t> active_sprites;
    std::deque<obj_sprite_t> displayed_sprites;

    u32 current_pixel = 0;

    bool
        vblank_int_requested = false,
        old_mode2_int = false,
        old_mode1_int = false,
        old_mode0_int = false;
};
