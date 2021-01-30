#include <cstring>
#include <iostream>

#include "gb_core/defs.hpp"
#include "gb_core/io_reg.hpp"
#include "gb_core/video.hpp"

#include "util/bit.hpp"
#include "util/util.hpp"

#define reg(X) (io->registers.X)

#define LCD_ENABLED          (Bit.test(reg(LCDC), 7))
#define WINDOW_TILE_MAP      (Bit.test(reg(LCDC), 6))
#define WINDOW_ENABLED       (Bit.test(reg(LCDC), 5))
#define BG_WND_TILE_DATA     (Bit.test(reg(LCDC), 4))
#define BG_TILE_MAP          (Bit.test(reg(LCDC), 3))
#define BIG_SPRITES          (Bit.test(reg(LCDC), 2))
#define OBJ_ENABLED          (Bit.test(reg(LCDC), 1))
#define BG_WND_ENABLED       (Bit.test(reg(LCDC), 0)) //TODO: DMG only

#define OBJ_BG_PRIORITY(obj) (Bit.test((obj).attrs, 7))
#define OBJ_Y_FLIP(obj)      (Bit.test((obj).attrs, 6))
#define OBJ_X_FLIP(obj)      (Bit.test((obj).attrs, 5))
#define OBJ_PALLETTE_1(obj)  (Bit.test((obj).attrs, 4)) //false if obj pallette 0

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

/**
 * TODO:
 * looks like the GB Die has finally been extracted so eventually it would be wise
 * to decode the PPU logic into here
 */
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

            memset(screen_buffer, 0xFF, GB_S_P_SZ);
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
        skip_fetch = true;

        if(first_line) {
            first_line = false;
        } else {
            y_line++;
        }

        x_line = 0;
        line_clock_count = 0;

        sprite_counter = 0;

        bg_fifo->clear();
        pause_bg_fifo = false;

        active_sprites.clear();
        displayed_sprites.clear();

        in_window = false;

        reg(LY) = y_line; //set LY

        if(y_line < 144) {
            //lock scroll for the current line
            x_sc = reg(SCX);
            y_sc = reg(SCY);

            process_step = SCANLINE_OAM;
            oam_fetch_step = OAM_0;
            vram_fetch_step = BM_0;

            bg_map_addr =
                (0x9800) |
                ((BG_TILE_MAP) ? 0x0400 : 0) |
                (((y_line+y_sc) & 0xf8) << 2) |
                (x_sc >> 3);

            wnd_map_addr =
                0x9800 |
                ((WINDOW_TILE_MAP) ? 0x0400 : 0) |
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
        case OAM_0:
            current_sprite = oam_fetch_sprite(sprite_counter);

            oam_fetch_step = OAM_1;
            break;

        // oam clk 2
        case OAM_1:
            if(active_sprites.size() < 10 &&
            current_sprite.pos_x > 0 &&
            y_line + 16 >= current_sprite.pos_y &&
            y_line + 16 < current_sprite.pos_y + ((BIG_SPRITES) ? 16 : 8)) {
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

            vram_fetch_step = BM_1;
            break;

        // Background map clk 2
        case BM_1:
            tile_addr = 0x8000;
            if(Bit.test(bg_map_byte, 7)) {
                tile_addr += 0x0800;
            }
            else if(!BG_WND_TILE_DATA) {
                tile_addr += 0x1000;
            }

            tile_addr +=
                    ((bg_map_byte & 0x7F) << 4) |
                    (((y_line + y_sc) & 0x7) << 1);

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
            if(Bit.test(wnd_map_byte, 7)) {
                tile_addr += 0x0800;
            }
            else if(!BG_WND_TILE_DATA) {
                tile_addr += 0x1000;
            }

            tile_addr +=
                    ((wnd_map_byte & 0x7F) << 4) |
                    (((y_line + y_sc) & 0x7) << 1);

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
            if(skip_fetch) {
                // According to the NGVT doc, we perform a fetch and
                // toss the result at the start of a new fetch sequence
                skip_fetch = false;

                vram_fetch_step = BM_0;
            } else {
                vram_fetch_step = SP_0;
            }
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
                obj_sprite_t curr_sprite = *(displayed_sprites.end() - 1);
                displayed_sprites.erase(displayed_sprites.end() - 1);

                s8 pixel_line = y_line - (curr_sprite.pos_y - 16);
                u8 tile_num = curr_sprite.tile_num & (BIG_SPRITES ? ~1 : ~0);

                if (OBJ_Y_FLIP(curr_sprite)) {
                    pixel_line = (BIG_SPRITES ? 15 : 7) - pixel_line;
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

            if(displayed_sprites.empty()) {
                pause_bg_fifo = false;

                // on the 8th clk of a BO1S cycle, shift in bg pixels,
                // this will no occur on subsequent sprite reads as the bg_fifo
                // should be disabled until the sprite fetches are done
                if(bg_fifo->size() == 0) {
                    for(int i = 0; i < 8; i++) {
                        u8 tile_idx = ((tile_byte_1 >> (7 - i)) & 1);
                        tile_idx   |= ((tile_byte_2 >> (7 - i)) & 1) << 1;
                        tile_idx *= 2;

                        u8 color = (reg(BGP) >> tile_idx) & 0x3;

                        bg_fifo->enqueue(color);
                    }

                    if(in_window) {
                        vram_fetch_step = WM_0;
                    }
                    else {
                        vram_fetch_step = BM_0;
                    }
                }
            } else {
                vram_fetch_step = SP_0;
            }
            break;

        default:
            std::cerr << "vram step error" << vram_fetch_step << std::endl;
        }

        /**
         * Actual PPU logic
         */
        if(bg_fifo->size() > 0 && !pause_bg_fifo) {
            u8 color_idx = bg_fifo->dequeue();

            //if background is disabled, force write a 0
            if(!BG_WND_ENABLED) {
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

            // if frame is disabled, don't draw pixel data
            if(!frame_disable) {
                memcpy(screen_buffer + current_byte, pixel_colors[color_idx], 3);
                current_byte += 3;
            }

            //after drawing a pixel, increase current x line
            x_line++;

            if(OBJ_ENABLED ) {
                for(auto sprite : active_sprites) {
                    if(x_line == sprite.pos_x - 8) {
                        displayed_sprites.push_back(sprite);
                    }
                }

                if(!displayed_sprites.empty()) {
                    pause_bg_fifo = true;
                }
            }

            if(WINDOW_ENABLED && x_line == (reg(WX) - 7) && !in_window) {
                std::cout << "starting window at: " << as_hex(x_line) << "," << as_hex(y_line) << ": map" << (WINDOW_TILE_MAP ? "0x9C00" : "0x9800") << " tile" << (BG_WND_TILE_DATA ? "0x8800-0x9000" : "0x8000-0x8800") << std::endl;
                in_window = true;
                vram_fetch_step = WM_1;
                bg_fifo->clear();
            } else if(x_line == 160) {
                process_step = HBLANK;
            }
        }

    /**
     * HBLANK
     */
    } else if(process_step == HBLANK) {
        bg_fifo->clear();
        sp_fifo->clear();
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