#pragma once

#include "cfg.hpp"
#include "io.hpp"

#include "defs.hpp"

#include "util/ints.hpp"

#include "util/CircularQueue.hpp"

#define OBG_ATTR_BG_PRIORITY   7
#define OBG_ATTR_Y_FLIP        6
#define OBG_ATTR_X_FLIP        5
#define OBG_ATTR_GB_PALLETTE   4
#define OBG_ATTR_GBC_TILE_BANK 3
#define OBG_ATTR_GBC_PALLETTE  0x7

#define STAT_COIN_INT_BIT   6
#define STAT_MODE_2_INT_BIT 5
#define STAT_MODE_1_INT_BIT 4
#define STAT_MODE_0_INT_BIT 3
#define STAT_COIN_BIT       2
#define STAT_MODE_FLAG      0x3

class Video_Controller {
public:
    enum {
        SCANLINE_OAM  = 2,
        SCANLINE_VRAM = 3,
        HBLANK        = 0,
        VBLANK        = 1,
    } v_mode_t;

    enum {
        TILE_MAP_1 = 0, //9800h-9BFFh
        TILE_MAP_2,     //9C00h-9FFFh
    } bg_tile_map_t;

    enum {
        MODE_0_1 = 1, //8000 method
        MODE_2_1 = 0, //8800 method
    } tile_addr_mode;

    typedef struct {
        u8 pos_y;
        u8 pos_x;
        u8 tile_num;
        u8 attrs;
    } obj_sprite_t;


    const u8 pixel_colors[4][3] = { //using html codes for now
        {0xFF, 0xFF, 0xFF}, // white
        {0xD3, 0xD3, 0xD3}, // lightgrey
        {0x69, 0x69, 0x69}, // dimgrey
        {0x00, 0x00, 0x00}  // black
    };

    Video_Controller(IO_Bus *io, Configuration *cfg, u8 *screen_buffer);
    ~Video_Controller();

    bool tick();

    obj_sprite_t oam_fetch_sprite(int index);
    bool ppu_tick();

private:
    IO_Bus *io;
    Configuration *cfg;

    bool gbc_mode;
    u8 *screen_buffer = NULL; //buffer for the screen, passed from core

    int curr_mode;

    // bool lcd_enabled;
    // int  window_tile_map;
    // bool window_enabled;
    // bool bg_wnd_tile_data;
    // int  bg_tile_map;
    // bool big_sprites;
    // bool obj_enabled;
    // bool bg_wnd_blank;
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
    CircularQueue<u8> *pix_fifo = new CircularQueue<u8>(160);
    u8 process_step;

    int frame_clock_count = 0; // count of clocks in a frame
    int line_clock_count = 0;  // count of clocks in a line

    enum __OAM_fetch_steps {
        OAM_1,
        OAM_2
    } oam_fetch_step;

    enum __VRAM_fetch_steps {
        BM_1, //Background Map clk 1
        BM_2, //Background Map clk 2

        WM_1, //Window Map clk 1
        WM_2, //Window Map clk 2;

        //TODO: Sprite fetch

        TD_0_0, //Tile Data byte 1 clk 1
        TD_0_1, //Tile Data byte 1 clk 2

        TD_1_0, //Tile Data byte 2 clk 1
        TD_1_1, //Tile Data byte 2 clk 2

        SP_0,   //Sprite 1
        SP_1,   //Sprite 2
    } vram_fetch_step = (__VRAM_fetch_steps)0;

    int ybase;

    u16
        bg_map_addr,    // addr of the current tile in the bg  tile map
        wnd_map_addr;   // addr of the current tile in the wnd tile map
    s16
        tile_addr;      // relative addr of the current tile data to fetch
    u8
        bg_map_byte,    // byte from the current tile in the bg  tile map
        wnd_map_byte,   // byte from the current tile in the wnd tile map
        tile_byte_1,    // 1st byte from the current tile fetched
        tile_byte_2;    // 1st byte from the current tile fetched

    u16
        y_line, // y counter for current_pixel
        x_line; // x counter for current pixel
    u16
        x_sc,   // x_scroll locked at beginning of line
        y_sc;   // y_scroll locked as beginning of line
    bool
        first_frame = false,   // first frame after lcd enable
        frame_disable = false, // disable pixel output for current frame(used with first frame)
        new_frame = false,     // the first clock tick of a new frame
        first_line = false,    // the first clock tick of the first line
        new_line = false,      // the first clock tick of a new line
        start_of_line = false;  // the first VRAM access of a new line


    int sprite_counter = 0;
    obj_sprite_t current_sprite;
    std::vector<obj_sprite_t> displayed_sprites;

    u32 current_byte = 0;

    //check that vblank int was requested this frame;
    bool vblank_int_requested = false;
};