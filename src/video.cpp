#include "video.hpp"

#include "io_reg.hpp"
#include "util/bit.hpp"

#include <cstring>

#define LCD_ENABLED      Bit.test(io->registers.LCDC, 7)
#define WINDOW_TILE_MAP  Bit.test(io->registers.LCDC, 6)
#define WINDOW_ENABLED   Bit.test(io->registers.LCDC, 5)
#define BG_WND_TILE_DATA Bit.test(io->registers.LCDC, 4)
#define BG_TILE_MAP      Bit.test(io->registers.LCDC, 3)
#define BIG_SPRITES      Bit.test(io->registers.LCDC, 2)
#define OBJ_ENABLED      Bit.test(io->registers.LCDC, 1)
#define BG_WND_BLANK     Bit.test(io->registers.LCDC, 0) //TODO: DMG only

Video_Controller::Video_Controller(IO_Bus *io, Configuration *cfg) :
io(io),
cfg(cfg) {}

Video_Controller::~Video_Controller() {}

bool Video_Controller::tick() {
    //dma_tick(); //TODO
    return ppu_tick();
}

/**
 * Internal Stuff
 */

Video_Controller::obj_sprite_t Video_Controller::oam_fetch_sprite(int index) {
    int loc = index*4;
    return (Video_Controller::obj_sprite_t){
        io->read_oam(loc + 0, true),
        io->read_oam(loc + 1, true),
        io->read_oam(loc + 2, true),
        io->read_oam(loc + 3, true)};
}

bool Video_Controller::ppu_tick() {
    /**
     * Occurs on Every Frame
     */
    if(new_frame) {
        new_frame = false;

        bg_map_addr = 0x9800;
        y_line = -1; //it will be incremeneted to 0 in the new_line block
        new_line = true;
    }

    /**
     * Occurs on Every line
     */
    if(new_line) {
        new_line = false;

        x_line = 0;
        line_clock_count = 0;

        registers.LY = y_line++; //set LY
        if(registers.LY == registers.LYC) {
            Bit.set(&registers.STAT, STAT_COIN_BIT);
        } else {
            Bit.reset(&registers.STAT, STAT_COIN_BIT);
        }

        x_sc = registers.SCX; //lock scx for the current line
        y_sc = registers.SCY; //lock scy for the current line
        process_step = SCANLINE_OAM;
        oam_fetch_step = OAM_1;

        bg_map_addr =
                (0x9800) |
                ((BG_TILE_MAP == TILE_MAP_1) ? 0 : 0x0400) |
                ((u16)(y_line & 0xf8) << 2) |
                (x_sc >> 3);
    }

    /**
     * Occurs on every clock
     */
    registers.STAT = (registers.STAT & ~STAT_MODE_FLAG) | process_step; //set current mode

    bool
        coin_int =  (Bit.test(registers.STAT, STAT_COIN_INT_BIT) && Bit.test(registers.STAT, STAT_COIN_BIT)),
        mode2_int = (Bit.test(registers.STAT, STAT_MODE_2_INT_BIT) && process_step == SCANLINE_OAM),
        mode1_int = (Bit.test(registers.STAT, STAT_MODE_1_INT_BIT) && process_step == VBLANK),
        mode0_int = (Bit.test(registers.STAT, STAT_MODE_0_INT_BIT) && process_step == HBLANK);

    if(coin_int || mode2_int || mode1_int || mode0_int) {
        io->registers.IF |= IO_Bus::Interrupt::LCD_STAT_INT;
    }

    /**
     * OAM Fetch Mode
     */
    if(process_step == SCANLINE_OAM) {
        switch(oam_fetch_step) {

        // oam clk 1
        case OAM_1:
            current_sprite = oam_fetch_sprite(sprite_counter++);

            oam_fetch_step = OAM_2;
            break;

        // oam clk 2
        case OAM_2:
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
            bg_map_byte = io->read_vram(bg_map_addr);

            bg_map_addr = (bg_map_addr & 0xFFE0) | ((bg_map_addr+1) & 0x001F);

            vram_fetch_step = BM_2;
            break;
        case BM_2:
            if(BG_WND_TILE_DATA == Video_Controller::MODE_0_1) {
                tile_addr =
                    (bg_map_byte << 4) |
                    ((ybase & 0x7) << 1);
            } else {
                tile_addr =
                    (0x1000 - (bg_map_byte << 4)) |
                    ((ybase & 0x7) << 1);
            }

            vram_fetch_step = TD_0_0;
            break;

        case WM_1:
        case WM_2:

        case TD_0_0:
            tile_byte = io->read_vram(tile_addr++);

            vram_fetch_step = TD_0_1;
            break;
        case TD_0_1:
            //TODO: check if this is backwards
            pix_fifo->enqueue(tile_byte & 0x03);
            pix_fifo->enqueue((tile_byte & 0x0c) >> 2);
            pix_fifo->enqueue((tile_byte & 0x30) >> 4);
            pix_fifo->enqueue((tile_byte & 0xc0) >> 6);

            vram_fetch_step = TD_1_0;
            break;
        case TD_1_0:
            tile_byte = io->read_vram(tile_addr++);

            vram_fetch_step = TD_1_1;
            break;
        case TD_1_1:
            //TODO: check if this is backwards
            pix_fifo->enqueue(tile_byte & 0x03);
            pix_fifo->enqueue((tile_byte & 0x0c) >> 2);
            pix_fifo->enqueue((tile_byte & 0x30) >> 4);
            pix_fifo->enqueue((tile_byte & 0xc0) >> 6);

            //TODO: logic for which step to go back to.
            vram_fetch_step = BM_1;
            break;
        }

        if(pix_fifo->size() > 0) {
            u8 pallette = registers.BGP;
            u8 color = (pallette >> pix_fifo->dequeue()) && 0x3;

            memcpy(&screen_buffer[current_byte], pixel_colors[color], 3);
            current_byte += 3;

            x_line++;
            if(x_line == 160) {
                process_step == HBLANK;
            }
        }
        else {
            std::cout << "FIFO Empty" << std::endl;
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
        //do nothing as the clock_count checker below will reset the frame

    /**
     * ERROR
     */
    } else {
        std::cout << "process_step error: " << vram_fetch_step << std::endl;
    }

    line_clock_count++; //used to find end of HBLANK

    frame_clock_count++;
    if(frame_clock_count >= 70223) {
        std::cout << "end frame" << std::endl;
        frame_clock_count = 0;
        new_frame = true;
        return true;
    } else {
        return false;
    }
}