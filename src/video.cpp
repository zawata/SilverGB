#include <cstring>

#include "defs.hpp"
#include "io_reg.hpp"
#include "video.hpp"

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

Video_Controller::Video_Controller(IO_Bus *io, Configuration *cfg, u8 *scrn_buf) :
io(io),
cfg(cfg),
screen_buffer(scrn_buf) {}

Video_Controller::~Video_Controller() {}

bool Video_Controller::tick() {
    //dma_tick(); //TODO
    return ppu_tick();
}

/**
 * Internal Stuff
 */

Video_Controller::obj_sprite_t Video_Controller::oam_fetch_sprite(int index) {
    int loc = 0xFE00 + (index * 4);
    return (Video_Controller::obj_sprite_t){
        io->read_oam(loc + 0, true),
        io->read_oam(loc + 1, true),
        io->read_oam(loc + 2, true),
        io->read_oam(loc + 3, true)};
}

bool Video_Controller::ppu_tick() {
    // std::cout << "ppu_tick: " << frame_clock_count << ", ";
    //TODO: How many of the PPU variables can we make static?

    if(!LCD_ENABLED) {
        // std::cout << "off" << std::endl;
        //TODO: detail any further behavior?
        //reset LY?

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
        if(reg(LY) == reg(LYC)) {
            Bit.set(&reg(STAT), STAT_COIN_BIT);
        } else {
            Bit.reset(&reg(STAT), STAT_COIN_BIT);
        }

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
    reg(STAT) = (reg(STAT) & ~STAT_MODE_FLAG) | process_step; //set current mode

    bool
        coin_int =  (Bit.test(reg(STAT), STAT_COIN_INT_BIT) && Bit.test(reg(STAT), STAT_COIN_BIT)),
        mode2_int = (Bit.test(reg(STAT), STAT_MODE_2_INT_BIT) && process_step == SCANLINE_OAM),
        mode1_int = (Bit.test(reg(STAT), STAT_MODE_1_INT_BIT) && process_step == VBLANK),
        mode0_int = (Bit.test(reg(STAT), STAT_MODE_0_INT_BIT) && process_step == HBLANK);

    if(coin_int || mode2_int || mode1_int || mode0_int) {
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
        case BM_1:
            bg_map_byte = io->read_vram(bg_map_addr, true);
            if(!start_of_line) {
                bg_map_addr = (bg_map_addr & 0xFFE0) | ((bg_map_addr+1) & 0x001F);
            }

            vram_fetch_step = BM_2;
            break;

        case BM_2:
            //std::cout << "B";
            tile_addr_base = 0x8000;
            tile_addr = (bg_map_byte << 4) | (((y_line+y_sc) & 0x7) << 1);

            if(BG_WND_TILE_DATA == Video_Controller::MODE_2_1) {
                tile_addr_base = 0x9000;
                tile_addr = (0x1000 - tile_addr);
            }

            vram_fetch_step = TD_0_0;
            break;

        case WM_1: //TODO

            vram_fetch_step = WM_2;
            break;

        case WM_2: //TODO

            vram_fetch_step = TD_0_0;
            break;

        case TD_0_0:
            tile_byte_1 = io->read_vram(tile_addr_base + tile_addr, true);
            tile_addr++;

            vram_fetch_step = TD_0_1;
            break;

        case TD_0_1:
            //std::cout << "0";
            vram_fetch_step = TD_1_0;
            break;

        case TD_1_0:
            tile_byte_2 = io->read_vram(tile_addr_base + tile_addr, true);
            tile_addr++;

            vram_fetch_step = TD_1_1;
            break;

        case TD_1_1:
            //std::cout << "1";

            if(start_of_line) {
                start_of_line = false;

                vram_fetch_step = BM_1;
            } else {
                vram_fetch_step = SP_0;
            }
            break;

        case SP_0:
            //TODO
            vram_fetch_step = SP_1;
            break;

        case SP_1:
            //TODO

            for(int i = 0; i < 8; i++) {
                u8 out_color = ((tile_byte_1 >> (7 - i)) & 1) << 1;
                out_color   |= ((tile_byte_2 >> (7 - i)) & 1);

                pix_fifo->enqueue(out_color);
            }

            vram_fetch_step = BM_1; //TODO: logic for which step to go back to.
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
            if(x_line == 160) {
                process_step = HBLANK;
            }
        }

    /**
     * HBLANK
     */
    } else if(process_step == HBLANK) {
        if(line_clock_count >= 456) {
            new_line = true;
        }

    /**
     * VBLANK
     */
    } else if(process_step == VBLANK) {
        if(!vblank_int_requested) {
            vblank_int_requested = true;
            io->request_interrupt(IO_Bus::VBLANK_INT);
        }
        //clock_count checker below will reset the frame

    /**
     * ERROR
     */
    } else {
        std::cerr << "process_step error: " << process_step << std::endl;
    }

    line_clock_count++; //used to find end of HBLANK
    frame_clock_count++;

    if(frame_clock_count >= 70223) { //70224 clks per frame
        frame_clock_count = 0;
        new_frame = true;
        return true;
    } else {
        return false;
    }
}