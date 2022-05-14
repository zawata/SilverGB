#include <cassert>
#include <cstring>

#include <nowide/iostream.hpp>

#include "gb_core/defs.hpp"
#include "gb_core/mem.hpp"
#include "gb_core/ppu.hpp"

#include "util/bit.hpp"
#include "util/util.hpp"

#define reg(X) (mem->registers.X)

#define LCDC_LCD_ENABLED_BIT      7
#define LCDC_WINDOW_TILE_MAP_BIT  6
#define LCDC_WINDOW_ENABLED_BIT   5
#define LCDC_BG_WND_TILE_DATA_BIT 4
#define LCDC_BG_TILE_MAP_BIT      3
#define LCDC_BIG_SPRITES_BIT      2
#define LCDC_OBJ_ENABLED_BIT      1
#define LCDC_BG_ENABLED_BIT       0

#define PRIORITY_BIT      7
#define Y_FLIP_BIT        6
#define X_FLIP_BIT        5
#define GB_PALLETTE_BIT   4
#define GBC_VRAM_BANK_BIT 3
#define GBC_PALLETTE_MASK 0x7

#define STAT_COIN_INT_BIT   6
#define STAT_MODE_2_INT_BIT 5
#define STAT_MODE_1_INT_BIT 4
#define STAT_MODE_0_INT_BIT 3
#define STAT_COIN_BIT       2
#define STAT_MODE_FLAG      (0x3)

#define LCDC_LCD_ENABLED       (Bit::test(reg(LCDC), LCDC_LCD_ENABLED_BIT))
#define LCDC_WINDOW_TILE_MAP   (Bit::test(reg(LCDC), LCDC_WINDOW_TILE_MAP_BIT))
#define LCDC_WINDOW_ENABLED    (Bit::test(reg(LCDC), LCDC_WINDOW_ENABLED_BIT))
#define LCDC_BG_WND_TILE_DATA  (Bit::test(reg(LCDC), LCDC_BG_WND_TILE_DATA_BIT))
#define LCDC_BG_TILE_MAP       (Bit::test(reg(LCDC), LCDC_BG_TILE_MAP_BIT))
#define LCDC_BIG_SPRITES       (Bit::test(reg(LCDC), LCDC_BIG_SPRITES_BIT))
#define LCDC_OBJ_ENABLED       (Bit::test(reg(LCDC), LCDC_OBJ_ENABLED_BIT))
#define LCDC_BG_ENABLED        (Bit::test(reg(LCDC), LCDC_BG_ENABLED_BIT))
#define LCDC_CGB_BG_PRIORITY   (LCDC_BG_ENABLED)

#define OBJ_PRIORITY(obj)      (Bit::test((obj).attrs, PRIORITY_BIT))
#define OBJ_Y_FLIP(obj)        (Bit::test((obj).attrs, Y_FLIP_BIT))
#define OBJ_X_FLIP(obj)        (Bit::test((obj).attrs, X_FLIP_BIT))
#define OBJ_GB_PALLETTE(obj)   (Bit::test((obj).attrs, GB_PALLETTE_BIT)) //false if obj pallette 0
#define OBJ_GBC_VRAM_BANK(obj) (Bit::test((obj).attrs, GBC_VRAM_BANK_BIT)) //false if bank 0
#define OBJ_GBC_PALLETTE(obj)  ((obj).attrs & GBC_PALLETTE_MASK)

#define BG_PRIORITY(attr)       (Bit::test((attr), PRIORITY_BIT))
#define BG_Y_FLIP(attr)         (Bit::test((attr), Y_FLIP_BIT))
#define BG_X_FLIP(attr)         (Bit::test((attr), X_FLIP_BIT))
#define BG_VRAM_BANK(attr)      (Bit::test((attr), GBC_VRAM_BANK_BIT)) //false if bank 0
#define BG_PALLETTE(attr)       ((attr) & GBC_PALLETTE_MASK)

void write_5bit_color(u8 *loc, u16 color) {
    u8 r = ((color >> 0)  & 0x001F);
    *loc++ = (r << 3) | (r >> 2);
    u8 g = ((color >> 5)  & 0x001F);
    *loc++ = (g << 3) | (g >> 2);
    u8 b = ((color >> 10) & 0x001F);
    *loc++ = (b << 3) | (b >> 2);
}

PPU::PPU(Memory *mem, u8 *scrn_buf, gb_device_t device, bool bootrom_enabled = false) :
mem(mem),
device(device),
screen_buffer(scrn_buf) {
    //TODO: demagic

    if(!bootrom_enabled) {
        nowide::cout << "Starting PPU without bootrom not supported!" << std::endl;
       new_frame = true;
       first_frame = true;

       frame_clock_count = 0;

       for(int i = 0; i < 8; i++) {
           bg_pallettes[i].colors[0] = 0x7F;
           bg_pallettes[i].colors[1] = 0x7F;
           bg_pallettes[i].colors[2] = 0x7F;
           bg_pallettes[i].colors[3] = 0x7F;
       }
    }

    wnd_enabled_bit = new Bit::BitWatcher<u8>(&reg(LCDC), LCDC_WINDOW_ENABLED_BIT);
}

PPU::~PPU() {}

bool PPU::tick() {
    return ppu_tick();
}

void PPU::set_color_data(u8 *reg, pallette_t *pallette_mem, u8 data) {
    bool high_byte = Bit::test(*reg, 0);
    u8 color_idx = (*reg & 0x6) >> 1;
    u8 pallette_idx = (*reg & 0x38) >> 3;

    if(Bit::test(*reg, 7)) {
        *reg = 0x80 | ((*reg + 1) & 0x3F);
    }

    // Deny color write if in mode 3
    if(process_step != SCANLINE_VRAM) {
        u16 *color = &pallette_mem[pallette_idx].colors[color_idx];

        if(high_byte) {
            *color = (*color & 0x00FF) | ((u16)data << 8);
        } else {
            *color = (*color & 0xFF00) | ((u16)data);
        }
    }
}

u8 PPU::get_color_data(u8 *reg, pallette_t *pallette_mem) {
    u8 byte_idx = *reg & 0x1;
    u8 color_idx = (*reg & 0x6) >> 1;
    u8 pallette_idx = (*reg & 0x38) >> 3;

    pallette_t pallette = pallette_mem[pallette_idx];

    return pallette.colors[color_idx] & (0xFF << (8 * byte_idx));
}

void PPU::write_bg_color_data(u8 data) {
    set_color_data(&reg(BCPS), bg_pallettes, data);
}

u8 PPU::read_bg_color_data() {
    return get_color_data(&reg(BCPS), bg_pallettes);
}

void PPU::write_obj_color_data(u8 data) {
    set_color_data(&reg(OCPS), obj_pallettes, data);
}

u8 PPU::read_obj_color_data() {
    return get_color_data(&reg(BCPS), bg_pallettes);
}

/**
 * Internal Stuff
 */

PPU::obj_sprite_t PPU::oam_fetch_sprite(int index) {
    int loc = 0xFE00 + (index * 4);

    PPU::obj_sprite_t o;
    return o = {
            mem->read_oam(loc + 0, true),
            mem->read_oam(loc + 1, true),
            mem->read_oam(loc + 2, true),
            mem->read_oam(loc + 3, true)};
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

    bool bank1 = dev_is_GBC(device) && OBJ_GBC_VRAM_BANK(curr_sprite);

    u8 sprite_tile_1 = mem->read_vram(addr, true, bank1),
       sprite_tile_2 = mem->read_vram(addr + 1, true, bank1);

    u8 pallette = !OBJ_GB_PALLETTE(curr_sprite) ? reg(OBP0) : reg(OBP1);

    for(int i = 0; i < 8; i++) {
        u8 pix_idx = i;
        if (OBJ_X_FLIP(curr_sprite)) {
            pix_idx = 7 - pix_idx;
        }

        u8 tile_idx = ((sprite_tile_1 >> (7 - pix_idx)) & 1);
        tile_idx   |= ((sprite_tile_2 >> (7 - pix_idx)) & 1) << 1;

        fifo_color_t color{};

        if(dev_is_GBC(device)) {
            color.pallette = &obj_pallettes[OBJ_GBC_PALLETTE(curr_sprite)];
            color.color_idx = tile_idx;
        } else {
            tile_idx <<= 1;
            color.pallette = const_cast<pallette_t *>(&gb_pallette);
            //TODO: we're special casing the object pallettes for GB. should we fix this?
            color.color_idx = static_cast<u8>((pallette >> tile_idx) & 0x3_u8);
        }

        color.is_transparent = tile_idx == 0;
        color.priority = !OBJ_PRIORITY(curr_sprite);

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

void PPU::ppu_tick_oam() {
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
}

void PPU::ppu_tick_vram() {
    if(skip_sprite_clock) {
        skip_sprite_clock = false;
        return;
    }

    if(pause_bg_fifo) {
        if(displayed_sprites.size() > 0) {
            enqueue_sprite_data(displayed_sprites.front());
            displayed_sprites.pop_front();
        } else {
            pause_bg_fifo = false;
        }

        skip_sprite_clock = true;
        return;
    }

    /**
     * VRAM fetches take 2 cycles to occur.
     *
     * to simplify logic, we use 2 enums per section
     */
    switch(vram_fetch_step) {
    // Background map clk 1
    case BM_0:
        bg_map_byte = mem->read_vram(bg_map_addr, true, false); // read from bank 0

        if(dev_is_GBC(device)) {
            attr_byte = mem->read_vram(bg_map_addr, true, true); //read from bank 1
            bg_map_bank_1 = BG_VRAM_BANK(attr_byte);
        } else {
            bg_map_bank_1 = false;
        }

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

        tile_y_line = ((y_cntr + y_sc) & 0x7);
        if(dev_is_GBC(device) && BG_Y_FLIP(attr_byte)) {
            tile_y_line = (7 - tile_y_line);
        }

        tile_addr += ((bg_map_byte & 0x7F) << 4) | (tile_y_line << 1);

        vram_fetch_step = TD_0_0;
        break;

    // window map clk 1
    case WM_0:
        wnd_map_byte = mem->read_vram(wnd_map_addr, true, false); // read from bank 0

        if(dev_is_GBC(device)) {
            attr_byte = mem->read_vram(wnd_map_addr, true, true); //read from bank 1
            bg_map_bank_1 = BG_VRAM_BANK(attr_byte);
        } else {
            bg_map_bank_1 = false;
        }

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

        tile_y_line = (wnd_y_cntr & 0x7);
        if(dev_is_GBC(device) && BG_Y_FLIP(attr_byte)) {
            tile_y_line = (7 - tile_y_line);
        }

        tile_addr += ((wnd_map_byte & 0x7F) << 4) | (tile_y_line << 1);

        vram_fetch_step = TD_0_0;
        break;

    // tile data 1 clk 1
    case TD_0_0:
        tile_byte_1 = mem->read_vram(tile_addr, true, bg_map_bank_1);
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
        tile_byte_2 = mem->read_vram(tile_addr, true, bg_map_bank_1);
        tile_addr++;

        vram_fetch_step = TD_1_1;
        break;

    // tile data 2 clk 2
    case TD_1_1:
        //Do nothing
        vram_fetch_step = IDLE;
        break;

    // Idle Clocking
    case IDLE:
        // shift in bg pixels when the bg fifo is empty.
        // this will not occur on subsequent sprite reads as the bg_fifo
        // should be disabled until the sprite fetches are done
        //TODO: this should be moved out of the idle clocking as it makes
        // the first fetch 2 clock cycles too long
        if(bg_fifo->size() == 0) {
            for(int i = 0; i < 8; i++) {
                u8 x_pixel = ((dev_is_GBC(device) && BG_X_FLIP(attr_byte)) ? (i) : (7 - i)) ;

                u8 tile_idx = ((tile_byte_1 >> x_pixel) & 1);
                tile_idx   |= ((tile_byte_2 >> x_pixel) & 1) << 1;

                fifo_color_t color{};

                if(dev_is_GBC(device)) {
                    color.priority = BG_PRIORITY(attr_byte);
                    color.pallette = &bg_pallettes[BG_PALLETTE(attr_byte)];
                    color.color_idx = tile_idx & 0x3_u8;
                } else {
                    tile_idx <<= 1;
                    color.priority = false;
                    color.pallette = const_cast<pallette_t *>(&gb_pallette);
                    color.color_idx = (reg(BGP) >> tile_idx) & 0x3_u8;
                }

                color.is_transparent = false;

                bg_fifo->enqueue(color);
            }

            if(in_window) {
                vram_fetch_step = WM_0;
            }
            else {
                vram_fetch_step = BM_0;
            }
        }
        break;

    default:
        //invalid VRAM fetch step
        nowide::cerr << "vram step error" << vram_fetch_step << std::endl;
        assert(false);
    }

    /**
     * Actual PPU logic
     */
    if(bg_fifo->size() > 0) {
        fifo_color_t bg_color;
        bg_fifo->dequeue(bg_color);

        //if background is disabled, force write a 0
        if(!LCDC_BG_ENABLED && !dev_is_GBC(device)) {
            bg_color.color_idx = 0;
        }


        //if drawing a sprite, dequeue a sprite pixel
        if(sp_fifo->size() > 0) {
            fifo_color_t s_color;
            sp_fifo->dequeue(s_color);

            //this could all be a single if check but it would get messy as hell
            bool bg_has_priority = dev_is_GBC(device) && bg_color.priority && LCDC_CGB_BG_PRIORITY;
            bool sprite_has_priority = s_color.priority && !bg_has_priority;
            bool should_draw_sprite = bg_color.color_idx == 0 || sprite_has_priority;

            if(!s_color.is_transparent && should_draw_sprite) {
                bg_color = s_color;
            }
        }

        //skip first 8 pixels and first pixels of partially offscreen tiles
        if(x_cntr >= 8 + (x_sc % 8)) {
            pix_clock_count++;

            // if frame is disabled, don't draw pixel data
            if (!frame_disable) {
                write_5bit_color(screen_buffer + current_byte, bg_color.pallette->colors[bg_color.color_idx]);
                current_byte += 3;
            }
        }


        if(LCDC_OBJ_ENABLED ) {
            for(auto sprite : active_sprites) {
                if(x_cntr - (x_sc % 8) == sprite.pos_x) {
                    pause_bg_fifo = true;
                    displayed_sprites.push_back(sprite);
                }
            }
        }

        if(!in_window) {
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

    if(wnd_enabled_bit->risen()) {
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

            memset(screen_buffer, 0xFF, GB_S_P_SZ); //TODO: use the white color
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
            nowide::cerr << "y_cntr OOB: " << y_cntr << std::endl;
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
            !coin_bit_signal                        &&
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
        mem->request_interrupt(Memory::Interrupt::LCD_STAT_INT);
    }

    if(process_step == SCANLINE_OAM) {
        ppu_tick_oam();
    } else if(process_step == SCANLINE_VRAM) {
        ppu_tick_vram();
    } else if(process_step == HBLANK) {
        if(line_clock_count >= 455) {
            new_line = true;
        }
    } else if(process_step == VBLANK) {
        if(!vblank_int_requested) {
            vblank_int_requested = true;
            mem->request_interrupt(Memory::Interrupt::VBLANK_INT);
        }
        //clock_count checker below will reset the frame

        if(line_clock_count >= 456) {
            //TODO: supposedly the first VBLANK is shorter than the rest? refer to TCAGBD and confirm
            new_line = true;
        }
    } else {
        nowide::cerr << "process_step error: " << as_hex((int)process_step) << std::endl;
    }

    line_clock_count++; //used in HBLANK and VBLANK
    frame_clock_count++;

    if(frame_clock_count < TICKS_PER_FRAME) {
        return false;
    } else {
        frame_clock_count = 0;
        new_frame = true;
        return true;
    }
}