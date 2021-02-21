#include <cassert>
#include <cstring>
#include <iostream>

#include "gb_core/defs.hpp"
#include "gb_core/io_reg.hpp"
#include "gb_core/ppu.hpp"

#include "util/bit.hpp"
#include "util/util.hpp"

#define reg(X) (io->registers.X)

#define LCDC_LCD_ENABLED_BIT      7
#define LCDC_WINDOW_TILE_MAP_BIT  6
#define LCDC_WINDOW_ENABLED_BIT   5
#define LCDC_BG_WND_TILE_DATA_BIT 4
#define LCDC_BG_TILE_MAP_BIT      3
#define LCDC_BIG_SPRITES_BIT      2
#define LCDC_OBJ_ENABLED_BIT      1
#define LCDC_BG_ENABLED_BIT   0

#define OBG_BG_PRIORITY_BIT   7
#define OBG_Y_FLIP_BIT        6
#define OBG_X_FLIP_BIT        5
#define OBG_GB_PALLETTE_BIT   4

#define STAT_COIN_INT_BIT   6
#define STAT_MODE_2_INT_BIT 5
#define STAT_MODE_1_INT_BIT 4
#define STAT_MODE_0_INT_BIT 3
#define STAT_COIN_BIT       2
#define STAT_MODE_FLAG      (0x3)

#define LCDC_LCD_ENABLED          (Bit::test(reg(LCDC), LCDC_LCD_ENABLED_BIT))
#define LCDC_WINDOW_TILE_MAP      (Bit::test(reg(LCDC), LCDC_WINDOW_TILE_MAP_BIT))
#define LCDC_WINDOW_ENABLED       (Bit::test(reg(LCDC), LCDC_WINDOW_ENABLED_BIT))
#define LCDC_BG_WND_TILE_DATA     (Bit::test(reg(LCDC), LCDC_BG_WND_TILE_DATA_BIT))
#define LCDC_BG_TILE_MAP          (Bit::test(reg(LCDC), LCDC_BG_TILE_MAP_BIT))
#define LCDC_BIG_SPRITES          (Bit::test(reg(LCDC), LCDC_BIG_SPRITES_BIT))
#define LCDC_OBJ_ENABLED          (Bit::test(reg(LCDC), LCDC_OBJ_ENABLED_BIT))
#define LCDC_BG_ENABLED           (Bit::test(reg(LCDC), LCDC_BG_ENABLED_BIT)) //TODO: DMG only

#define OBJ_BG_PRIORITY(obj) (Bit::test((obj).attrs, OBG_BG_PRIORITY_BIT))
#define OBJ_Y_FLIP(obj)      (Bit::test((obj).attrs, OBG_Y_FLIP_BIT))
#define OBJ_X_FLIP(obj)      (Bit::test((obj).attrs, OBG_X_FLIP_BIT))
#define OBJ_PALLETTE_1(obj)  (Bit::test((obj).attrs, OBG_GB_PALLETTE_BIT)) //false if obj pallette 0

PPU::PPU(IO_Bus *io, u8 *scrn_buf, bool bootrom_enabled = false) :
io(io),
screen_buffer(scrn_buf) {
    if(!bootrom_enabled) {
        std::cout << "Starting PPU without bootrom not supported!" << std::endl;
       new_frame = true;
       first_frame = true;

       frame_clock_count = 0;
    }

    wnd_enabled_bit = new Bit::BitWatcher<u8>(&reg(LCDC), LCDC_WINDOW_ENABLED_BIT);
}

PPU::~PPU() {}

bool PPU::tick() {
    io->dma_tick();
    return ppu_tick();
}

/**
 * Internal Stuff
 */

PPU::obj_sprite_t PPU::oam_fetch_sprite(int index) {
    int loc = 0xFE00 + (index * 4);

    PPU::obj_sprite_t o;
    return o = {
            io->read_oam(loc + 0, true),
            io->read_oam(loc + 1, true),
            io->read_oam(loc + 2, true),
            io->read_oam(loc + 3, true)};
}

void PPU::enqueue_sprite_data(PPU::obj_sprite_t const& curr_sprite) {
    s8 pixel_line = y_cntr - (curr_sprite.pos_y - 16);
    u8 tile_num = curr_sprite.tile_num & (LCDC_BIG_SPRITES ? ~1 : ~0);

    if (OBJ_Y_FLIP(curr_sprite)) {
        pixel_line = (LCDC_BIG_SPRITES ? 15 : 7) - pixel_line;
    }

    u16 addr = 0x8000 |
                (tile_num << 4) |
                (pixel_line << 1);

    u8 sprite_tile_1 = io->read_vram(addr, true),
        sprite_tile_2 = io->read_vram(addr + 1, true);

    u8 pallette = !OBJ_PALLETTE_1(curr_sprite) ? reg(OBP0) : reg(OBP1);

    for(int i = 0; i < 8; i++) {
        u8 pix_idx = i;
        if (OBJ_X_FLIP(curr_sprite)) {
            pix_idx = 7 - pix_idx;
        }

        u8 tile_idx = ((sprite_tile_1 >> (7 - pix_idx)) & 1);
        tile_idx   |= ((sprite_tile_2 >> (7 - pix_idx)) & 1) << 1;
        tile_idx *= 2;

        sprite_fifo_color_t color{};
        color.color_idx = static_cast<u8>((pallette >> tile_idx) & 0x3_u8);
        color.is_transparent = tile_idx == 0;
        color.bg_priority = OBJ_BG_PRIORITY(curr_sprite);

        if(sp_fifo->size() > i) {
            //lower idx sprites have priority, only replace pixels
            // if one is transparent
            if(!color.is_transparent && sp_fifo->at(i).is_transparent) {
                sp_fifo->replace(i, color);
            }
        }
        else {
            sp_fifo->enqueue(color);
        }
    }
}

bool PPU::ppu_tick() {
/**
 * TODO:
 *  - Looks like the GB Die has finally been extracted so eventually it would be wise
 * to decode the PPU logic into here
 *
 *  - How many of the PPU variables can we make static?
 */

    if(!LCDC_LCD_ENABLED) {
        //TODO: detail any further behavior?
        reg(LY) = 0;
        reg(STAT) &= ~STAT_MODE_FLAG; //clear mode bits

        new_frame = true;
        first_frame = true;

        frame_clock_count = 0;

        return false;
    }

    //check if video registers were written too:
    /**TODO: we only do this in this way because we're trying to avoid cyclic dependencies and ppu has to access IO members.
     *  this relationship can be reversed with the following changes:
     * - moving interrupt handling to either the CPU or it's own class
     * - move VRAM storage to PPU
     * - find someway to continue to tick the DMA, maybe put it in either misc ticking or cpu.
     **/
    if(wnd_enabled_bit->risen()) {
        std::cout << "wnd_enabled rising edge" << std::endl;
        wnd_y_cntr = 0;
        wnd_map_addr =
            0x9800 |
            ((LCDC_WINDOW_TILE_MAP) ? 0x0400 : 0) |
            wnd_y_cntr << 2;
    }

    /**
     * Occurs on Every Frame
     */
    if(new_frame) {
        new_frame = false;

        if(first_frame) {
            first_frame = false;
            frame_disable = true;

            memset(screen_buffer, 0xFF, GB_S_P_SZ);
        } else {
            frame_disable = false;
        }

        current_byte = 0;

        wnd_y_cntr = 0;
        y_cntr = 0;
        new_line = true;
        first_line = true;

        vblank_int_requested = false;
    }

    /**
     * Occurs on Every line
     */
    if(new_line) {
        new_line = false;
        skip_fetch = true;

        if(first_line) {
            first_line = false;
        } else {
            y_cntr++;
        }

        x_cntr = 0;
        pix_clock_count = 0;
        line_clock_count = 0;

        sprite_counter = 0;
        active_sprites.clear();
        displayed_sprites.clear();

        bg_fifo->clear();
        pause_bg_fifo = false;

        in_window = false;

        reg(LY) = y_cntr; //set LY

        if(y_cntr < 144) {
            //lock scroll for the current line
            x_sc = reg(SCX);
            y_sc = reg(SCY);

            process_step = SCANLINE_OAM;
            oam_fetch_step = OAM_0;
            vram_fetch_step = BM_0;

            bg_map_addr =
                (0x9800) |
                ((LCDC_BG_TILE_MAP) ? 0x0400 : 0) |
                (((y_cntr+y_sc) & 0xf8) << 2) |
                (x_sc >> 3);

            wnd_map_addr =
                0x9800 |
                ((LCDC_WINDOW_TILE_MAP) ? 0x0400 : 0) |
                (wnd_y_cntr & 0xf8) << 2;

        } else if(y_cntr < 154) {
            process_step = VBLANK;
        } else {
            std::cerr << "y_cntr OOB: " << y_cntr << std::endl;
        }
    }

    /**
     * Occurs on every clock
     */
    if(frame_disable && y_cntr == 0 && process_step == SCANLINE_OAM) {
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
        Bit::set(&reg(STAT), STAT_COIN_BIT);
    } else {
        Bit::reset(&reg(STAT), STAT_COIN_BIT);
    }

    bool coin_int =
            Bit::test(reg(STAT), STAT_COIN_INT_BIT) &&
            Bit::test(reg(STAT), STAT_COIN_BIT)     &&
            !coin_bit_signal                       &&
            old_LY != reg(LY);
    old_LY  = reg(LY);
    coin_bit_signal = Bit::test(reg(STAT), STAT_COIN_BIT);

    bool
        mode2_int = (Bit::test(reg(STAT), STAT_MODE_2_INT_BIT) && process_step == SCANLINE_OAM),
        mode1_int = (Bit::test(reg(STAT), STAT_MODE_1_INT_BIT) && process_step == VBLANK),
        mode0_int = (Bit::test(reg(STAT), STAT_MODE_0_INT_BIT) && process_step == HBLANK),
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
        case OAM_0:
            current_sprite = oam_fetch_sprite(sprite_counter);

            oam_fetch_step = OAM_1;
            break;

        // oam clk 2
        case OAM_1:
            if(active_sprites.size() < 10 &&
            current_sprite.pos_x > 0 &&
            y_cntr + 16 >= current_sprite.pos_y &&
            y_cntr + 16 < current_sprite.pos_y + ((LCDC_BIG_SPRITES) ? 16 : 8)) {
                active_sprites.push_back(current_sprite);
            }

            sprite_counter++;
            oam_fetch_step = OAM_0;
            break;
        }

        //quit after scanning all 40 sprites
        if(sprite_counter == 40) {
            //reuse sprite_counter for rendering
            sprite_counter = 0;
            process_step = SCANLINE_VRAM;
        }

    /**
     * VRAM Fetch Mode
     */
    } else if(process_step == SCANLINE_VRAM) {
        /**
         * VRAM fetches take 2 cycles to occur.
         *
         * to simplify logic, we use 2 enums per section
         */
        switch(vram_fetch_step) {

        // Background map clk 1
        case BM_0:
            bg_map_byte = io->read_vram(bg_map_addr, true);

            //increment bg_map_addr, but only the bottom 5 bits.
            // this will wrap around the edge of the tile map
            if(!skip_fetch) {
                bg_map_addr = (bg_map_addr & 0xFFE0) | ((bg_map_addr+1) & 0x001F);
            }
            skip_fetch = false;

            vram_fetch_step = BM_1;
            break;

        // Background map clk 2
        case BM_1:
            tile_addr = 0x8000;
            if(Bit::test(bg_map_byte, 7)) {
                tile_addr += 0x0800;
            }
            else if(!LCDC_BG_WND_TILE_DATA) {
                tile_addr += 0x1000;
            }

            tile_addr +=
                    ((bg_map_byte & 0x7F) << 4) |
                    (((y_cntr + y_sc) & 0x7) << 1);

            vram_fetch_step = TD_0_0;
            break;

        // window map clk 1
        case WM_0:
            wnd_map_byte = io->read_vram(wnd_map_addr, true);
            wnd_map_addr++;

            vram_fetch_step = WM_1;
            break;

        // window map clk 2
        case WM_1:
            tile_addr = 0x8000;
            if(Bit::test(wnd_map_byte, 7)) {
                tile_addr += 0x0800;
            }
            else if(!LCDC_BG_WND_TILE_DATA) {
                tile_addr += 0x1000;
            }

            tile_addr +=
                    ((wnd_map_byte & 0x7F) << 4) |
                    ((wnd_y_cntr & 0x7) << 1);

            vram_fetch_step = TD_0_0;
            break;

        // tile data 1 clk 1
        case TD_0_0:
            tile_byte_1 = io->read_vram(tile_addr, true);
            tile_addr++;

            vram_fetch_step = TD_0_1;
            break;

        // tile data 1 clk 2
        case TD_0_1:
            //Do nothing
            vram_fetch_step = TD_1_0;
            break;

        // tile data 2 clk 1
        case TD_1_0:
            tile_byte_2 = io->read_vram(tile_addr, true);
            tile_addr++;

            vram_fetch_step = TD_1_1;
            break;

        // tile data 2 clk 2
        case TD_1_1:
            //Do nothing
            vram_fetch_step = SP_0;
            break;

        // sprite data clk 1
        // Doubles as IDLE
        case SP_0:
            //Do nothing for the sake of simplicity
            vram_fetch_step = SP_1;
            break;

        // sprite data clk 2
        case SP_1:
            if(displayed_sprites.size() > 0) {
                enqueue_sprite_data(displayed_sprites.front());
                displayed_sprites.pop_front();

                vram_fetch_step = SP_0;
            } else {
                pause_bg_fifo = false;

                // shift in bg pixels when the bg fifo is empty.
                // this will not occur on subsequent sprite reads as the bg_fifo
                // should be disabled until the sprite fetches are done
                //TODO: this should be moved out of the idle clocking as it makes
                // the first fetch 2 clock cycles too long
                if(bg_fifo->size() == 0) {
                    for(int i = 0; i < 8; i++) {
                        u8 tile_idx = ((tile_byte_1 >> (7 - i)) & 1);
                        tile_idx   |= ((tile_byte_2 >> (7 - i)) & 1) << 1;
                        tile_idx *= 2;

                        bg_fifo->enqueue((reg(BGP) >> tile_idx) & 0x3);
                    }

                    if(in_window) {
                        vram_fetch_step = WM_0;
                    }
                    else {
                        vram_fetch_step = BM_0;
                    }
                }
            }
            break;

        default:
            //invalid VRAM fetch step
            std::cerr << "vram step error" << vram_fetch_step << std::endl;
            assert(false);
        }

        /**
         * Actual PPU logic
         */
        if(bg_fifo->size() > 0 && !pause_bg_fifo) {
            u8 color_idx = bg_fifo->dequeue();

            //if background is disabled, force write a 0
            if(!LCDC_BG_ENABLED) {
                color_idx = 0;
            }

            //if drawing a sprite, dequeue a sprite pixel
            if(sp_fifo->size() > 0) {
                sprite_fifo_color_t s_color = sp_fifo->dequeue();

                //if the color in the sprite pixel isn't transparent
                // or has priority, replace the background color
                if(!s_color.is_transparent) {
                    if(!s_color.bg_priority || color_idx == 0) {
                        color_idx = s_color.color_idx;
                    }
                }
            }

            if (x_cntr > 8 + (x_sc & 0x7_u8)) {
                pix_clock_count++;

                // if frame is disabled, don't draw pixel data
                if (!frame_disable) {
                    memcpy(screen_buffer + current_byte, pixel_colors[color_idx], 3);
                    current_byte += 3;
                }
            }


            if(LCDC_OBJ_ENABLED ) {
                for(auto sprite : active_sprites) {
                    if(x_cntr == sprite.pos_x) {
                        pause_bg_fifo = true;
                        displayed_sprites.push_back(sprite);
                    }
                }
            }

            if (!in_window) {
                if (LCDC_WINDOW_ENABLED && x_cntr >= reg(WX) && y_cntr >= reg(WY)) {
                    in_window = true;
                    vram_fetch_step = WM_0;
                    bg_fifo->clear();
                }
            }

            if(pix_clock_count == 160) {
                if (in_window) {
                    wnd_y_cntr++;
                }
                process_step = HBLANK;
            }

            x_cntr++;
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