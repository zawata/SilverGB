#pragma once

#include <deque>

#include "io.hpp"

#include "defs.hpp"

#include "util/bit.hpp"
#include "util/ints.hpp"
#include "util/CircularQueue.hpp"

class PPU {
public:
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

    enum {
        color_id_WHITE = 0,
        color_id_LIGHTGREY,
        color_id_DARKGREY,
        color_id_BLACK,
        color_id_TRANSPARENT,
    } color_id_t;

    union sprite_fifo_color_t {
        struct {
            u8   color_idx      : 6;
            bool is_transparent : 1;
            bool bg_priority    : 1;
        };
        u8 byte;
    };
    static_assert(sizeof(sprite_fifo_color_t) == 1, "");

    //using html codes for now
    // TODO: make this user-configurable
    static constexpr u8 pixel_colors[6][3] = {
        {0xFF, 0xFF, 0xFF}, // white
        {0xD3, 0xD3, 0xD3}, // lightgrey
        {0x69, 0x69, 0x69}, // dimgrey
        {0x00, 0x00, 0x00}, // black
    };

    //green version
    // static constexpr u8 pixel_color[4][3] = {
    //     {0xE1, 0xF7, 0xD1}, // white
    //     {0x87, 0xC3, 0x72}, // lightgrey
    //     {0x33, 0x70, 0x53}, // dimgrey
    //     {0x09, 0x20, 0x21}, // black
    // };

    PPU(IO_Bus *io, u8 *scrn_buf, bool bootrom_enabled);
    ~PPU();

    bool tick();

    obj_sprite_t oam_fetch_sprite(int index);
    void enqueue_sprite_data(PPU::obj_sprite_t const& curr_sprite);

    bool ppu_tick();
    void ppu_tick_oam();
    void ppu_tick_vram();

private:
    IO_Bus *io;

    bool gbc_mode;
    u8 *screen_buffer = NULL; //buffer for the screen, passed from core

    int curr_mode;

    struct __registers {
        u8 LCDC;
        u8 STAT;
        u8 SCY;
        u8 SCX;
        u8 LY;
        u8 LYC;
        u8 DMA;
        u8 BGP;
        u8 OBP0;
        u8 OBP1;
        u8 WY;
        u8 WX;

        u8 VBK;
    } registers;

    /**
     * PPU Variables
     */
    CircularQueue<u8> *bg_fifo                  = new CircularQueue<u8>(160);
    CircularQueue<sprite_fifo_color_t> *sp_fifo = new CircularQueue<sprite_fifo_color_t>(160);
    u8 process_step;

    Bit::BitWatcher<u8> *wnd_enabled_bit;

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
        skip_sprite_clock = false;

    u16
        bg_map_addr,    // addr of the current tile in the bg  tile map
        wnd_map_addr,   // addr of the current tile in the wnd tile map
        tile_addr;      // relative addr of the current tile data to fetch

    u8
        bg_map_byte,    // byte from the current tile in the bg tile map
        wnd_map_byte,   // byte from the current tile in the wnd tile map
        tile_byte_1,    // 1st byte from the current tile fetched
        tile_byte_2;    // 1st byte from the current tile fetched

    u8
        wnd_y_cntr, // window y counter
        y_cntr, // y counter
        x_cntr, // x counter
        x_sc,   // x_scroll locked at beginning of line
        y_sc;   // y_scroll locked as beginning of line

    bool
        first_frame = false,   // first frame after lcd enable
        frame_disable = false, // disable pixel output for current frame(used with first frame)
        new_frame = false,     // the first clock tick of a new frame
        first_line = false,    // the first clock tick of the first line
        new_line = false,      // the first clock tick of a new line
        skip_fetch = false,    // the first VRAM access of a new line
        in_window = false;     // are we in window mode

    int sprite_counter = 0;
    obj_sprite_t current_sprite;
    std::deque<obj_sprite_t> active_sprites;
    std::deque<obj_sprite_t> displayed_sprites;

    u32 current_byte = 0;

    bool
        vblank_int_requested = false,
        old_mode2_int = false,
        old_mode1_int = false,
        old_mode0_int = false;
};