#include <cstring>
#include <iostream>

#include "gb_core/defs.hpp"
#include "gb_core/io_reg.hpp"
#include "gb_core/video.hpp"

#include "util/bit.hpp"

#define reg(X) (io->registers.X)

#define LCD_ENABLED      (Bit.test(reg(LCDC), 7))
#define WINDOW_TILE_MAP  (Bit.test(reg(LCDC), 6))
#define WINDOW_ENABLED   (Bit.test(reg(LCDC), 5))
#define BG_WND_TILE_DATA (Bit.test(reg(LCDC), 4))
#define BG_TILE_MAP      (Bit.test(reg(LCDC), 3))
#define BIG_SPRITES      (Bit.test(reg(LCDC), 2))
#define OBJ_ENABLED      (Bit.test(reg(LCDC), 1))
#define BG_WND_BLANK     (Bit.test(reg(LCDC), 0)) //TODO: DMG only

Video_Controller::Video_Controller(IO_Bus *io, u8 *scrn_buf, bool bootrom_enabled = false) :
io(io),
screen_buffer(scrn_buf) {
    if(!bootrom_enabled) {
        std::cout << "Starting PPU without bootrom not supported!" << std::endl;
       new_frame = true;
       first_frame = true;

       frame_clock_count = 0;
    }
}

Video_Controller::~Video_Controller() {}

bool Video_Controller::tick() {
    io->dma_tick();
    return ppu_tick();
}

/**
 * Internal Stuff
 */

Video_Controller::obj_sprite_t Video_Controller::oam_fetch_sprite(int index) {
    int loc = 0xFE00 + (index * 4);

    Video_Controller::obj_sprite_t o;
    return o = {
            io->read_oam(loc + 0, true),
            io->read_oam(loc + 1, true),
            io->read_oam(loc + 2, true),
            io->read_oam(loc + 3, true)};
}

bool Video_Controller::ppu_tick() {
    //TODO: How many of the PPU variables can we make static?

    if(!LCD_ENABLED) {
        //TODO: detail any further behavior?
        reg(LY) = 0;
        reg(STAT) &= ~STAT_MODE_FLAG; //clear mode bits

        new_frame = true;
        first_frame = true;

        frame_clock_count = 0;

        return false;
    }

    /**
     * Occurs on Every Frame
     */
    if(new_frame) {
        new_frame = false;

        if(first_frame) {
            first_frame = false;
            frame_disable = true;

            // memset(screen_buffer, 0xFF, GB_S_P_SZ);
        } else {
            frame_disable = false;
        }

        current_byte = 0;

        bg_map_addr = 0x9800;
        y_line = 0;
        new_line = true;
        first_line = true;

        vblank_int_requested = false;
    }

    /**
     * Occurs on Every line
     */
    if(new_line) {
        new_line = false;
        start_of_line = true;

        if(first_line) {
            first_line = false;
        } else {
            y_line++;
        }

        x_line = 0;
        line_clock_count = 0;

        sprite_counter = 0;

        pix_fifo->clear();

        reg(LY) = y_line; //set LY

        if(y_line < 144) {
            x_sc = reg(SCX); //lock scx for the current line
            y_sc = reg(SCY); //lock scy for the current line

            process_step = SCANLINE_OAM;
            oam_fetch_step = OAM_1;
            vram_fetch_step = BM_1;

            bg_map_addr =
                (0x9800) |
                ((BG_TILE_MAP == TILE_MAP_1) ? 0 : 0x0400) |
                ((u16)((y_line+y_sc) & 0xf8) << 2) |
                (x_sc >> 3);

            wnd_map_addr =
                0x9800 |
                ((WINDOW_TILE_MAP == TILE_MAP_1) ? 0 : 0x0400) |
                ((y_line + y_sc) & 0xf8) << 2;

        } else if(y_line < 154) {
            process_step = VBLANK;
            //TODO: anything else?
        } else {
            std::cerr << "y_line OOB: " << y_line << std::endl;
        }
    }

    /**
     * Occurs on every clock
     */
    if(frame_disable && y_line == 0 && process_step == SCANLINE_OAM) {
        //frame disable is true for the first frame after the LCD is reenabled.

        /**
         * TODO: verify
         * Allegedly, or at least as far as BGB and gekkio's tests indicate,
         * The first frame after the LCD is reenabled, during the OAM state
         * the STAT mode flag is set to 0 instead of 2. It's corrected during
         * the transition to VRAM mode.
         **/
        reg(STAT) &= ~STAT_MODE_FLAG;
    } else {
        reg(STAT) = (reg(STAT) & ~STAT_MODE_FLAG) | process_step; //set current mode
    }

    /**
     * TODO: verify
     * STAT Coin Bit sets instantaneously, but stat-IF isn't set until LY changes
     */
    if(reg(LY) == reg(LYC)) {
        Bit.set(&reg(STAT), STAT_COIN_BIT);
    } else {
        Bit.reset(&reg(STAT), STAT_COIN_BIT);
    }

    bool coin_int =
            Bit.test(reg(STAT), STAT_COIN_INT_BIT) &&
            Bit.test(reg(STAT), STAT_COIN_BIT)     &&
            !coin_bit_signal                       &&
            old_LY != reg(LY);
    old_LY  = reg(LY);
    coin_bit_signal = Bit.test(reg(STAT), STAT_COIN_BIT);

    bool
        mode2_int = (Bit.test(reg(STAT), STAT_MODE_2_INT_BIT) && process_step == SCANLINE_OAM),
        mode1_int = (Bit.test(reg(STAT), STAT_MODE_1_INT_BIT) && process_step == VBLANK),
        mode0_int = (Bit.test(reg(STAT), STAT_MODE_0_INT_BIT) && process_step == HBLANK),
        throw_stat =
            coin_int                                    ||
            (mode2_int && (mode2_int != old_mode2_int)) ||
            (mode1_int && (mode1_int != old_mode1_int)) ||
            (mode0_int && (mode0_int != old_mode0_int));
    old_mode2_int = mode2_int;
    old_mode1_int = mode1_int;
    old_mode0_int = mode0_int;
    if(throw_stat) {
        io->request_interrupt(IO_Bus::Interrupt::LCD_STAT_INT);
    }

    /**
     * OAM Fetch Mode
     */
    if(process_step == SCANLINE_OAM) {
        switch(oam_fetch_step) {

        // oam clk 1
        case OAM_1:
            current_sprite = oam_fetch_sprite(sprite_counter);

            oam_fetch_step = OAM_2;
            break;

        // oam clk 2
        case OAM_2:
            sprite_counter++;
            if(displayed_sprites.size() < 10 && current_sprite.pos_x &&
               y_line + 16 >= current_sprite.pos_y &&
               y_line + 16 < current_sprite.pos_y + ((BIG_SPRITES) ? 16 : 8))
            {
                displayed_sprites.push_back(current_sprite);
            }

            oam_fetch_step = OAM_1;
            break;
        }

        //quit after scanning all 40 sprites
        if(sprite_counter == 40) {
            process_step = SCANLINE_VRAM;
        }

    /**
     * VRAM Fetch Mode
     */
    } else if(process_step == SCANLINE_VRAM) {
        /**
         * VRAM fetches take 2 cycles to occur.
         *
         * to make sure we aren't just losing a cycle, we can perform the
         * access in the first cycle and the processing in the second.
         * hence every fetch step has 2 enum entries.
         */
        switch(vram_fetch_step) {

        // Background map clk 1
        case BM_1:
            bg_map_byte = io->read_vram(bg_map_addr, true);

            //increment bg_map_addr, but only the bottom 5 bits.
            // this will wrap around the edge of the tile map
            if(!start_of_line) {
                bg_map_addr = (bg_map_addr & 0xFFE0) | ((bg_map_addr+1) & 0x001F);
            }

            vram_fetch_step = BM_2;
            break;

        // Background map clk 2
        case BM_2:
            tile_addr = 0;
            if(Bit.test(bg_map_byte, 7)) tile_addr = 0x800;
            else if(BG_WND_TILE_DATA == Video_Controller::MODE_2_1) tile_addr = 0x1000;

            tile_addr +=
                    ((bg_map_byte & 0x7F) << 4) |
                    (((y_line + y_sc) & 0x7) << 1);

            vram_fetch_step = TD_0_0;
            break;

        // window map clk 1
        case WM_1:

            wnd_map_byte = io->read_vram(wnd_map_addr, true);
            wnd_map_addr++;

            vram_fetch_step = WM_2;
            break;

        // window map clk 2
        case WM_2:
            tile_addr = 0;
            if(Bit.test(wnd_map_byte, 7)) tile_addr = 0x800;
            else if(BG_WND_TILE_DATA == Video_Controller::MODE_2_1) tile_addr = 0x1000;

            tile_addr +=
                    ((wnd_map_byte & 0x7F) << 4) |
                    (((y_line + y_sc) & 0x7) << 1);

            vram_fetch_step = TD_0_0;
            break;

        // tile data 1 clk 1
        case TD_0_0:
            tile_byte_1 = io->read_vram(0x8000 + tile_addr, true);
            tile_addr++;

            vram_fetch_step = TD_0_1;
            break;

        // tile data 1 clk 2
        case TD_0_1:
            vram_fetch_step = TD_1_0;
            break;

        // tile data 2 clk 1
        case TD_1_0:
            tile_byte_2 = io->read_vram(0x8000 + tile_addr, true);
            tile_addr++;

            vram_fetch_step = TD_1_1;
            break;

        // tile data 2 clk 2
        case TD_1_1:

            if(start_of_line) {
                start_of_line = false;

                vram_fetch_step = BM_1;
            } else {
                vram_fetch_step = SP_0;
            }
            break;

        // sprite data clk 1
        case SP_0:
            vram_fetch_step = SP_1;
            break;

        // sprite data clk 2
        case SP_1:

            for(int i = 0; i < 8; i++) {
                u8 out_color = ((tile_byte_1 >> (7 - i)) & 1);
                out_color   |= ((tile_byte_2 >> (7 - i)) & 1) << 1;


                pix_fifo->enqueue(out_color);
            }
            vram_fetch_step = BM_1;
            break;
        default:
            std::cerr << "vram step error" << vram_fetch_step << std::endl;
        }

        if(pix_fifo->size() > 0) {
            u8 pallette = reg(BGP);
            u8 pallette_index = pix_fifo->dequeue() * 2;
            u8 color = (pallette >> pallette_index) & 0x3;

            if(!frame_disable) {
                memcpy(screen_buffer + current_byte, pixel_colors[color], 3);
                current_byte += 3;
            }

            x_line++;
            if(x_line == reg(WX)) {
                vram_fetch_step = WM_1;
                pix_fifo->clear();
            } else if(x_line == 160) {
                process_step = HBLANK;
            }
        }

    /**
     * HBLANK
     */
    } else if(process_step == HBLANK) {
        if(line_clock_count >= 455) {
            new_line = true;
        }

    /**
     * VBLANK
     */
    } else if(process_step == VBLANK) {
        if(!vblank_int_requested) {
            vblank_int_requested = true;
            io->request_interrupt(IO_Bus::Interrupt::VBLANK_INT);
        }
        //clock_count checker below will reset the frame

        if(line_clock_count >= 456) {
            //TODO: supposedly the first VBLANK is shorter than the rest? refer to TCAGBD and confirm
            new_line = true;
        }

    /**
     * ERROR
     */
    } else {
        std::cerr << "process_step error: " << as_hex((int)process_step) << std::endl;
    }

    line_clock_count++; //used in HBLANK and VBLANK
    frame_clock_count++;

    if(frame_clock_count >= 70223) { //70224 clks per frame
        frame_clock_count = 0;
        new_frame = true;
        return true;
    } else {
        return false;
    }
}