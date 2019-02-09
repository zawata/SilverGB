#include "video.hpp"

#include "io_reg.hpp"
#include "util/bit.hpp"

#include <cstring>

Video_Controller::Video_Controller(bool gbc_mode) {
    if(gbc_mode)
        video_ram_char.reserve(GBC_VRAM_CHAR_SIZE);
    else
        video_ram_char.reserve(DMG_VRAM_CHAR_SIZE);
    video_ram_back.reserve(VRAM_BACK_SIZE);
    oam_ram.reserve(OAM_RAM_SIZE);
}

Video_Controller::~Video_Controller() {}

bool Video_Controller::tick() {
    //dma_tick(); //TODO
    return ppu_tick();
}

u8 Video_Controller::read_reg(u8 loc) {
    switch(loc) {
    case LCDC_REG:
        return registers.LCDC & LCDC_READ_MASK;
    case STAT_REG:
        return registers.STAT & STAT_READ_MASK;
    case SCY_REG:
        return registers.SCY  & SCY_READ_MASK;
    case SCX_REG:
        return registers.SCX  & SCX_READ_MASK;
    case LY_REG:
        return registers.LY   & LY_READ_MASK;
    case LYC_REG:
        return registers.LYC  & LYC_READ_MASK;
    case DMA_REG:
        return registers.DMA  & DMA_READ_MASK;
    case BGP_REG:
        return registers.BGP  & BGP_READ_MASK;
    case OBP0_REG:
        return registers.OBP0 & OBP0_READ_MASK;
    case OBP1_REG:
        return registers.OBP1 & OBP1_READ_MASK;
    case WY_REG:
        return registers.WY   & WY_READ_MASK;
    case WX_REG:
        return registers.WX   & WX_READ_MASK;
    case VBK_REG:
        return registers.VBK  | 0xFE; //all bits but bit0 one are set to 1
    }
}

void Video_Controller::write_reg(u8 loc, u8 data) {
    switch(loc) {
    case LCDC_REG:
        registers.LCDC = data & LCDC_WRITE_MASK;

        // LCD Display Enable
        if(Bit.test(data, 7)) {
            if(curr_mode == VBLANK) std::cerr << "LCDC.7 on VBLANK" << std::endl;
            else lcd_enabled = false;
        } else {
            lcd_enabled = true;
        }

        // Window Tile Map Display Select
        if(Bit.test(data, 6)) {
            window_tile_map = TILE_MAP_2;
        } else {
            window_tile_map = TILE_MAP_1;
        }

        // Window Display Enable
        window_enabled = Bit.test(data, 5);

        // BG & Window Tile Data Select
        if(Bit.test(data, 4)) {
            bg_wnd_tile_data = MODE_0_1;
        } else {
            bg_wnd_tile_data = MODE_2_1;
        }

        // BG Tile Map Display Select
        if(Bit.test(data, 3)) {
            bg_tile_map = TILE_MAP_2;
        } else {
            bg_tile_map = TILE_MAP_1;
        }

        // OBJ Size
        big_sprites = Bit.test(data, 2);

        // OBJ Display Enable
        obj_enabled = Bit.test(data, 1);

        //TODO: this is only for DMG
        bg_wnd_blank = Bit.test(data, 0); // BG/Window Display/Priority

        break;
    case STAT_REG:
        registers.STAT = data & STAT_WRITE_MASK;
        break;
    case SCY_REG:
        registers.SCY  = data & SCY_WRITE_MASK;
        break;
    case SCX_REG:
        registers.SCX  = data & SCX_WRITE_MASK;
        break;
    case LY_REG:
        registers.LY   = data & LY_WRITE_MASK;
        break;
    case LYC_REG:
        registers.LYC  = data & LYC_WRITE_MASK;
        break;
    case DMA_REG:
        registers.DMA  = data & DMA_WRITE_MASK;
        break;
    case BGP_REG:
        registers.BGP  = data & BGP_WRITE_MASK;
        break;
    case OBP0_REG:
        registers.OBP0 = data & OBP0_WRITE_MASK;
        break;
    case OBP1_REG:
        registers.OBP1 = data & OBP1_WRITE_MASK;
        break;
    case WY_REG:
        registers.WY   = data & WY_WRITE_MASK;
        break;
    case WX_REG:
        registers.WX   = data & WX_WRITE_MASK;
        break;
    case VBK_REG:
        if(gbc_mode) {
            registers.VBK = data & VBK_WRITE_MASK;
        }
        break;
    }
}

/**
 * Only the first half of the vram is banked so the simplest way to
 * store data and process requests to and from it IMO is to separate VRAM
 * into 2 parts, character data and background data.
 *
 * The character data exists entirely in the VRAM banks while the background
 * data is unbanked.
 *
 * If a request comes in less than the bank size then we check the vram bank vector
 * and if it's higher then it goes to the unbanked background vector
 *
 * OAM is always unbanked and is just a simple read/write operation;
 *
 * ==============================================================================
 * The bypass flag is used for debugging and allows checking OAM and VRAM outside
 * of VBLANK and HBLANK
 *
 * TODO: do the writes need a bypass too?
 **/
u8 Video_Controller::read_vram(u16 loc, bool bypass = false) {
    if(curr_mode == SCANLINE_VRAM && !bypass) return 0xFF;

    if(loc < VRAM_BANK_SIZE) {
        if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
            return video_ram_char[VRAM_BANK_SIZE + loc];
        }
        else {
            return video_ram_char[loc];
        }
    }
    else {
        return video_ram_back[loc-VRAM_BANK_SIZE];
    }
}

void Video_Controller::write_vram(u16 loc, u8 data) {
    if(loc < VRAM_BANK_SIZE) {
        if(gbc_mode && (registers.VBK & VBK_READ_MASK)) {
            video_ram_char[VRAM_BANK_SIZE + loc] = data;
        }
        else {
            video_ram_char[loc] = data;
        }
    }
    else {
        video_ram_back[loc-VRAM_BANK_SIZE] = data;
    }
}

u8 Video_Controller::read_oam(u16 loc, bool bypass = false) {
    if((curr_mode == SCANLINE_OAM || curr_mode == SCANLINE_VRAM) && !bypass) return 0xFF;

    return oam_ram[loc];
}

void Video_Controller::write_oam(u16 loc, u8 data) {
    oam_ram[loc] = data;
}

/**
 * Internal Stuff
 */

Video_Controller::obj_sprite_t Video_Controller::oam_fetch_sprite(int index) {
    int loc = index*4;
    return (Video_Controller::obj_sprite_t){
        read_oam(loc + 0, true),
        read_oam(loc + 1, true),
        read_oam(loc + 2, true),
        read_oam(loc + 3, true)};
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
                ((bg_tile_map == TILE_MAP_1) ? 0 : 0x0400) |
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
        io->registers.IF |= IO_Bus::Interrupt::LCD_STAT_INT
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
               y_line + 16 < current_sprite.pos_y + ((big_sprites) ? 16 : 8))
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
            bg_map_byte = read_vram(bg_map_addr);

            bg_map_addr = (bg_map_addr & 0xFFE0) | ((bg_map_addr+1) & 0x001F);

            vram_fetch_step = BM_2;
            break;
        case BM_2:
            if(bg_wnd_tile_data == Video_Controller::MODE_0_1) {
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
            tile_byte = read_vram(tile_addr++);

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
            tile_byte = read_vram(tile_addr++);

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