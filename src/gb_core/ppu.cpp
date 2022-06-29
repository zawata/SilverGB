#include "gb_core/ppu.hpp"

#include <cassert>
#include <cstring>
#include <nowide/iostream.hpp>

#include "gb_core/cart.hpp"
#include "gb_core/defs.hpp"
#include "gb_core/mem.hpp"

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
#define GB_PALETTE_BIT    4
#define GBC_VRAM_BANK_BIT 3
#define GBC_PALETTE_MASK  0x7

#define STAT_COIN_INT_BIT   6
#define STAT_MODE_2_INT_BIT 5
#define STAT_MODE_1_INT_BIT 4
#define STAT_MODE_0_INT_BIT 3
#define STAT_COIN_BIT       2
#define STAT_MODE_FLAG      (0x3)

#define LCDC_LCD_ENABLED      (Bit::test(reg(LCDC), LCDC_LCD_ENABLED_BIT))
#define LCDC_WINDOW_TILE_MAP  (Bit::test(reg(LCDC), LCDC_WINDOW_TILE_MAP_BIT))
#define LCDC_WINDOW_ENABLED   (Bit::test(reg(LCDC), LCDC_WINDOW_ENABLED_BIT))
#define LCDC_BG_WND_TILE_DATA (Bit::test(reg(LCDC), LCDC_BG_WND_TILE_DATA_BIT))
#define LCDC_BG_TILE_MAP      (Bit::test(reg(LCDC), LCDC_BG_TILE_MAP_BIT))
#define LCDC_BIG_SPRITES      (Bit::test(reg(LCDC), LCDC_BIG_SPRITES_BIT))
#define LCDC_OBJ_ENABLED      (Bit::test(reg(LCDC), LCDC_OBJ_ENABLED_BIT))
#define LCDC_BG_ENABLED       (Bit::test(reg(LCDC), LCDC_BG_ENABLED_BIT))
#define LCDC_CGB_BG_PRIORITY  (LCDC_BG_ENABLED)

#define OBJ_PRIORITY(obj)      (Bit::test((obj).attrs, PRIORITY_BIT))
#define OBJ_Y_FLIP(obj)        (Bit::test((obj).attrs, Y_FLIP_BIT))
#define OBJ_X_FLIP(obj)        (Bit::test((obj).attrs, X_FLIP_BIT))
#define OBJ_GB_PALETTE(obj)    (Bit::test((obj).attrs, GB_PALETTE_BIT))    // false if obj palette 0
#define OBJ_GBC_VRAM_BANK(obj) (Bit::test((obj).attrs, GBC_VRAM_BANK_BIT)) // false if bank 0
#define OBJ_GBC_PALETTE(obj)   ((obj).attrs & GBC_PALETTE_MASK)

#define BG_PRIORITY(attr)  (Bit::test((attr), PRIORITY_BIT))
#define BG_Y_FLIP(attr)    (Bit::test((attr), Y_FLIP_BIT))
#define BG_X_FLIP(attr)    (Bit::test((attr), X_FLIP_BIT))
#define BG_VRAM_BANK(attr) (Bit::test((attr), GBC_VRAM_BANK_BIT)) // false if bank 0
#define BG_PALETTE(attr)   ((attr)&GBC_PALETTE_MASK)

constexpr u16 rgb555_to_rgb15(u8 r, u8 g, u8 b) {
    return r | (((u16)g) << 5) | (((u16)b) << 10);
}

void rgb15_to_rgb555(u8 *loc, u16 color) {
    *loc++ = ((color >> 0) & 0x001F);
    *loc++ = ((color >> 5) & 0x001F);
    *loc++ = ((color >> 10) & 0x001F);
}

void rgb15_to_rgb888(u8 *loc, u16 color) {
    u8 r   = ((color >> 0) & 0x001F);
    *loc++ = (r << 3) | (r >> 2);
    u8 g   = ((color >> 5) & 0x001F);
    *loc++ = (g << 3) | (g >> 2);
    u8 b   = ((color >> 10) & 0x001F);
    *loc++ = (b << 3) | (b >> 2);
}

constexpr u8 inverse_title_checksums[] = {
        0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x4d, 0x00, 0x00, 0x23, 0x00,
        0x00, 0x00, 0x16, 0x1d, 0x02, 0x22, 0x49, 0x13, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x47, 0x43, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x14,
        0x03, 0x00, 0x00, 0x24, 0x00, 0x00, 0x07, 0x0a, 0x0e, 0x3f, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x42, 0x00,
        0x00, 0x28, 0x00, 0x20, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x11,
        0x00, 0x00, 0x0b, 0x3c, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x3e, 0x2b, 0x12, 0x4b, 0x40,
        0x00, 0x3d, 0x00, 0x1c, 0x0f, 0x39, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x01, 0x00, 0x00, 0x2d, 0x08, 0x00, 0x00, 0x00,
        0x21, 0x00, 0x09, 0x00, 0x00, 0x19, 0x00, 0x1f, 0x00, 0x1a, 0x35, 0x00, 0x3a, 0x38, 0x00, 0x00, 0x00, 0x00,
        0x27, 0x00, 0x00, 0x44, 0x00, 0x00, 0x15, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41,
        0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x45, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x04, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x00, 0x06, 0x00, 0x4e, 0x00, 0x26, 0x25, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x1e,
};

#define AMBIGUOUS_CHK_IDXS_START 0x41_u8
#define AMBIGUOUS_CHK_IDXS_END   0x4e_u8 // inclusive

constexpr char title_fourth_chars[][4] =
        {"BUR", "ER", "FA", "AR", "A ", "RI", "BN", "EA", "KI", "EL", "KI", " C", "RE", "- "};

#define _CVT(shf, idx) ((shf) << 5 | (idx))
constexpr u8 palette_triplet_ids_and_flags[] = {
        _CVT(0x3, 0x1c), _CVT(0x0, 0x08), _CVT(0x0, 0x12), _CVT(0x5, 0x03), _CVT(0x5, 0x02), _CVT(0x0, 0x07),
        _CVT(0x4, 0x07), _CVT(0x2, 0x0b), _CVT(0x1, 0x00), _CVT(0x0, 0x12), _CVT(0x3, 0x05), _CVT(0x5, 0x08),
        _CVT(0x0, 0x16), _CVT(0x5, 0x09), _CVT(0x4, 0x06), _CVT(0x5, 0x11), _CVT(0x3, 0x08), _CVT(0x5, 0x00),
        _CVT(0x4, 0x07), _CVT(0x3, 0x06), _CVT(0x0, 0x12), _CVT(0x5, 0x01), _CVT(0x1, 0x10), _CVT(0x1, 0x1c),
        _CVT(0x0, 0x12), _CVT(0x4, 0x05), _CVT(0x0, 0x12), _CVT(0x3, 0x04), _CVT(0x0, 0x1b), _CVT(0x0, 0x07),
        _CVT(0x0, 0x06), _CVT(0x3, 0x0f), _CVT(0x3, 0x0e), _CVT(0x3, 0x0e), _CVT(0x5, 0x0e), _CVT(0x5, 0x0f),
        _CVT(0x3, 0x0f), _CVT(0x5, 0x12), _CVT(0x5, 0x0f), _CVT(0x5, 0x12), _CVT(0x5, 0x08), _CVT(0x5, 0x0b),
        _CVT(0x3, 0x0f), _CVT(0x5, 0x0f), _CVT(0x4, 0x06), _CVT(0x5, 0x0e), _CVT(0x5, 0x02), _CVT(0x5, 0x02),
        _CVT(0x0, 0x12), _CVT(0x5, 0x0f), _CVT(0x0, 0x13), _CVT(0x0, 0x12), _CVT(0x5, 0x01), _CVT(0x3, 0x0e),
        _CVT(0x5, 0x0f), _CVT(0x5, 0x0f), _CVT(0x5, 0x0d), _CVT(0x0, 0x06), _CVT(0x2, 0x0c), _CVT(0x3, 0x0e),
        _CVT(0x5, 0x0f), _CVT(0x5, 0x0f), _CVT(0x0, 0x12), _CVT(0x3, 0x1c), _CVT(0x5, 0x0c), _CVT(0x5, 0x08),
        _CVT(0x3, 0x0a), _CVT(0x3, 0x0e), _CVT(0x0, 0x13), _CVT(0x5, 0x00), _CVT(0x1, 0x0d), _CVT(0x5, 0x08),
        _CVT(0x1, 0x0b), _CVT(0x5, 0x0c), _CVT(0x3, 0x04), _CVT(0x5, 0x0c), _CVT(0x3, 0x0d), _CVT(0x4, 0x07),
        _CVT(0x5, 0x1c), _CVT(0x3, 0x00), _CVT(0x5, 0x14), _CVT(0x0, 0x13), _CVT(0x3, 0x12), _CVT(0x3, 0x1c),
        _CVT(0x5, 0x15), _CVT(0x5, 0x0e), _CVT(0x5, 0x0e), _CVT(0x3, 0x1c), _CVT(0x3, 0x1c), _CVT(0x3, 0x05),
        _CVT(0x5, 0x02), _CVT(0x3, 0x0c), _CVT(0x3, 0x04), _CVT(0x4, 0x05),
};
#undef _CVT

constexpr u8 triplet_palette_idxs[][3] = {
        {16, 22,  8},
        {17,  4, 13},
        {32,  0, 14},
        {32,  4, 15},
        { 4,  4,  7},
        { 4, 22, 18},
        { 4, 22, 20},
        {28, 22, 24},
        {19, 31,  9},
        {16, 28, 10},
        {30, 30, 11},
        { 4, 23, 28},
        {17, 22,  2},
        { 4,  0,  2},
        { 4, 28,  3},
        {28,  3,  0},
        { 3, 28,  4},
        {21, 28,  4},
        { 3, 28,  0},
        { 4,  3, 27},
        {25,  3, 28},
        { 0, 28,  8},
        { 5,  5,  5},
        { 3, 28, 12},
        { 4,  3, 28},
        { 0,  0,  1},
        {28,  3,  6},
        {26, 26, 26},
        { 4, 28, 29}
};

#define rgb(...) rgb555_to_rgb15(__VA_ARGS__)
constexpr PPU::palette_t compat_palettes[] = {
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x15, 0x0C), rgb(0x10, 0x06, 0x00), rgb(0x00, 0x00, 0x00)}, /* 00 */
        {rgb(0x1F, 0x1C, 0x18), rgb(0x19, 0x13, 0x10), rgb(0x10, 0x0D, 0x05), rgb(0x0B, 0x06, 0x01)}, /* 01 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x11, 0x11, 0x1B), rgb(0x0A, 0x0A, 0x11), rgb(0x00, 0x00, 0x00)}, /* 02 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x0F, 0x1F, 0x06), rgb(0x00, 0x10, 0x00), rgb(0x00, 0x00, 0x00)}, /* 03 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x10, 0x10), rgb(0x12, 0x07, 0x07), rgb(0x00, 0x00, 0x00)}, /* 04 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x14, 0x14, 0x14), rgb(0x0A, 0x0A, 0x0A), rgb(0x00, 0x00, 0x00)}, /* 05 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x1F, 0x00), rgb(0x0F, 0x09, 0x00), rgb(0x00, 0x00, 0x00)}, /* 06 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x0F, 0x1F, 0x00), rgb(0x16, 0x0E, 0x00), rgb(0x00, 0x00, 0x00)}, /* 07 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x15, 0x15, 0x10), rgb(0x08, 0x0E, 0x0F), rgb(0x00, 0x00, 0x00)}, /* 08 */
        {rgb(0x14, 0x13, 0x1F), rgb(0x1F, 0x1F, 0x00), rgb(0x00, 0x0C, 0x00), rgb(0x00, 0x00, 0x00)}, /* 09 */
        {rgb(0x1F, 0x1F, 0x19), rgb(0x0C, 0x1D, 0x1D), rgb(0x13, 0x10, 0x06), rgb(0x0B, 0x0B, 0x0B)}, /* 10 */
        {rgb(0x16, 0x16, 0x1F), rgb(0x1F, 0x1F, 0x12), rgb(0x15, 0x0B, 0x08), rgb(0x00, 0x00, 0x00)}, /* 11 */
        {rgb(0x1F, 0x1F, 0x14), rgb(0x1F, 0x12, 0x12), rgb(0x12, 0x12, 0x1F), rgb(0x00, 0x00, 0x00)}, /* 12 */
        {rgb(0x1F, 0x1F, 0x13), rgb(0x12, 0x16, 0x1F), rgb(0x0C, 0x12, 0x0E), rgb(0x00, 0x07, 0x07)}, /* 13 */
        {rgb(0x0D, 0x1F, 0x00), rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x0A, 0x09), rgb(0x00, 0x00, 0x00)}, /* 14 */
        {rgb(0x0A, 0x1B, 0x00), rgb(0x1F, 0x10, 0x00), rgb(0x1F, 0x1F, 0x00), rgb(0x1F, 0x1F, 0x1F)}, /* 15 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x0E, 0x00), rgb(0x12, 0x08, 0x00), rgb(0x00, 0x00, 0x00)}, /* 16 */
        {rgb(0x1F, 0x18, 0x08), rgb(0x1F, 0x1A, 0x00), rgb(0x12, 0x07, 0x00), rgb(0x09, 0x00, 0x00)}, /* 17 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x0A, 0x1F, 0x00), rgb(0x1F, 0x08, 0x00), rgb(0x00, 0x00, 0x00)}, /* 18 */
        {rgb(0x1F, 0x0C, 0x0A), rgb(0x1A, 0x00, 0x00), rgb(0x0C, 0x00, 0x00), rgb(0x00, 0x00, 0x00)}, /* 19 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x13, 0x00), rgb(0x1F, 0x00, 0x00), rgb(0x00, 0x00, 0x00)}, /* 20 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x00, 0x1F, 0x00), rgb(0x06, 0x10, 0x00), rgb(0x00, 0x09, 0x00)}, /* 21 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x0B, 0x17, 0x1F), rgb(0x1F, 0x00, 0x00), rgb(0x00, 0x00, 0x1F)}, /* 22 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x1F, 0x0F), rgb(0x00, 0x10, 0x1F), rgb(0x1F, 0x00, 0x00)}, /* 23 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x1F, 0x00), rgb(0x1F, 0x00, 0x00), rgb(0x00, 0x00, 0x00)}, /* 24 */
        {rgb(0x1F, 0x1F, 0x00), rgb(0x1F, 0x00, 0x00), rgb(0x0C, 0x00, 0x00), rgb(0x00, 0x00, 0x00)}, /* 25 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x19, 0x00), rgb(0x13, 0x0C, 0x00), rgb(0x00, 0x00, 0x00)}, /* 26 */
        {rgb(0x00, 0x00, 0x00), rgb(0x00, 0x10, 0x10), rgb(0x1F, 0x1B, 0x00), rgb(0x1F, 0x1F, 0x1F)}, /* 27 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x0C, 0x14, 0x1F), rgb(0x00, 0x00, 0x1F), rgb(0x00, 0x00, 0x00)}, /* 28 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x0F, 0x1F, 0x06), rgb(0x00, 0x0C, 0x18), rgb(0x00, 0x00, 0x00)}, /* 29 */
  // straddled palettes get their own entries
        {rgb(0x00, 0x00, 0x00), rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x10, 0x10), rgb(0x12, 0x07, 0x07)}, /* 30 */
        {rgb(0x00, 0x00, 0x1F), rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x1F, 0x0F), rgb(0x00, 0x10, 0x1F)}, /* 31 */
        {rgb(0x1F, 0x1F, 0x1F), rgb(0x1F, 0x1F, 0x1F), rgb(0x0C, 0x14, 0x1F), rgb(0x00, 0x00, 0x1F)}, /* 32 */
};
#undef rgb

PPU::PPU(Cartridge *cart, Memory *mem, u8 *scrn_buf, gb_device_t device, bool bootrom_enabled = false) :
        cart(cart), mem(mem), device(device), screen_buffer(scrn_buf) {
    // TODO: demagic

    // Set object priority defaults, GBC will flip this later for dmg-compat mode if applicable
    // or we'll do it if there emulating the bootrom
    if(dev_is_GB(device)) {
        obj_priority_mode = true;
    } else if(dev_is_GBC(device)) {
        obj_priority_mode = false;
    }

    bool cgb_mode = dev_is_GBC(device) && cart->isCGBCart();

    if(!bootrom_enabled) {
        nowide::cout << "Starting PPU without bootrom not fully supported!" << std::endl;

        if(dev_is_GB(device)) {
            bg_palettes[0]  = gb_palette;
            obj_palettes[0] = gb_palette;
        } else if(dev_is_GBC(device)) {
            if(cart->isCGBCart()) {
                for(int i = 0; i < 8; i++) {
                    bg_palettes[i] = {0x7F, 0x7F, 0x7F, 0x7F};
                }
            } else {
                // set dmg-style object priority
                obj_priority_mode = true;
                mem->set_dmg_compat_mode(true);

                calculate_gbc_compat_palette();
            }
        }
    }

    new_frame   = true;
    first_frame = true;

    frame_clock_count = 0;
}

PPU::~PPU() {
    delete bg_fifo;
    delete sp_fifo;
}

void PPU::write_bg_color_data(u8 data) {
    set_color_data(&reg(BCPS), bg_palettes, data);
}

u8 PPU::read_bg_color_data() {
    return get_color_data(&reg(BCPS), bg_palettes);
}

void PPU::write_obj_color_data(u8 data) {
    set_color_data(&reg(OCPS), obj_palettes, data);
}

u8 PPU::read_obj_color_data() {
    return get_color_data(&reg(BCPS), bg_palettes);
}

void PPU::set_obj_priority(bool obj_has_priority) {
    // true on DMG and CGB Compat Mode
    //  false on CGB
    // TODO: we don't currently use this flag. fix that
    obj_priority_mode = obj_has_priority;
}

/**
 * Private
 */

void PPU::set_color_data(u8 *reg, palette_t *palette_mem, u8 data) {
    /**
     * (O/B)CPS Register Bit Format
     * IPPPPCCB
     *
     * I - incremenet after write
     * P - palette number(out of 16)
     * C - color number( out of 4)
     * B - high or low byte of color
     * */

    bool high_byte   = Bit::test(*reg, 0);
    u8   color_idx   = (*reg & 0x6) >> 1;
    u8   palette_idx = (*reg & 0x38) >> 3;

    if(Bit::test(*reg, 7)) {
        *reg = 0x80 | ((*reg + 1) & 0x3F);
    }

    // Deny color write if in mode 3
    if(process_step != SCANLINE_VRAM) {
        u16 *color = &palette_mem[palette_idx].colors[color_idx];

        if(high_byte) {
            *color = (*color & 0x00FF) | ((u16)data << 8);
        } else {
            *color = (*color & 0xFF00) | ((u16)data);
        }
    }
}

u8 PPU::get_color_data(u8 *reg, palette_t *palette_mem) {
    u8 byte_idx    = *reg & 0x1;
    u8 color_idx   = (*reg & 0x6) >> 1;
    u8 palette_idx = (*reg & 0x38) >> 3;

    palette_t palette = palette_mem[palette_idx];

    return (palette.colors[color_idx] >> (8 * byte_idx)) & 0xFF;
}

void PPU::calculate_gbc_compat_palette() {
    /**
     * Setup DMG Compatibility Mode palettes
     *
     * 🌠 Shit to know 🌠:
     *  - TODO: talk about title hashing.
     *
     *  - The palettes are organized into triplets(above)
     *  - triplets are 3 indexes into the palette color table.
     *  - But because Nintendo®, they also introduced "palette shuffling flags"
     *    - this is why palette_triplet_id isn't just an ID into the triplet table (or why we can't just do
     *      away with the palette_triplet_ids_and_flags)
     *  - in each triplet:
     *    - BGP is always the last index
     *    - OBP0 is either the first or last index depending on the first shuffle bit
     *    - OBP1 is any index depending on the value of the second and third bit.
     **/

    u8 compat_plt_id = 0;
    if(cart->cartSupportsGBCCompatMode()) {
        compat_plt_id = inverse_title_checksums[cart->computeTitleChecksum()];

        // if this title has an "ambiguous checksum", we need to further
        // differentiate by the 4th character of the cart title
        if(bounded(compat_plt_id, AMBIGUOUS_CHK_IDXS_START, AMBIGUOUS_CHK_IDXS_END)) {
            char cart_char = cart->getCartTitle()[3];

            // get the letter list that corresponds to this index and search it for the 4th char in the cart
            // title
            const char *found_char = strchr(title_fourth_chars[compat_plt_id - AMBIGUOUS_CHK_IDXS_START], cart_char);
            // if the char title is in the list
            if(found_char != NULL) {
                // offset the palette index based on which index the letter was found.
                auto x = title_fourth_chars[compat_plt_id - AMBIGUOUS_CHK_IDXS_START] - found_char;
                compat_plt_id += x * (AMBIGUOUS_CHK_IDXS_END - AMBIGUOUS_CHK_IDXS_START);
            } else {
                // otherwise use the default palette
                compat_plt_id = 0;
            }
        }
    }

    // decode palette id
    u8 palette_triplet_id = palette_triplet_ids_and_flags[compat_plt_id];
    u8 shuffle_flags      = palette_triplet_id >> 5;
    const u8(*triplet)[3] = &(triplet_palette_idxs[palette_triplet_id & 0x1F]);

    palette_t OBP0, OBP1;
    // theres probably a cleaner way to implement this logic but this is easier
    switch(shuffle_flags & 0x3) {
    // remember, OBP0 = 0, OBP1 = 1 in both the triplet and the obj_palette, BGP = 2 in the triplet.
    case 0b000:
        obj_palettes[0] = compat_palettes[(*triplet)[2]]; // same as BGP
        obj_palettes[1] = compat_palettes[(*triplet)[2]]; // same as BGP
        break;
    case 0b001:
        obj_palettes[0] = compat_palettes[(*triplet)[0]];
        obj_palettes[1] = compat_palettes[(*triplet)[2]]; // same as BGP
        break;
    case 0b010:
        obj_palettes[0] = compat_palettes[(*triplet)[2]]; // same as BGP
        obj_palettes[1] = compat_palettes[(*triplet)[0]]; // same as OBP0
        break;
    case 0b011:
        obj_palettes[0] = compat_palettes[(*triplet)[0]];
        obj_palettes[1] = compat_palettes[(*triplet)[0]]; // same as OBP0
        break;
    case 0b100:
        obj_palettes[0] = compat_palettes[(*triplet)[2]]; // same as BGP
        obj_palettes[1] = compat_palettes[(*triplet)[1]];
        break;
    case 0b101:
        obj_palettes[0] = compat_palettes[(*triplet)[0]];
        obj_palettes[1] = compat_palettes[(*triplet)[1]];
        break;
    // these cases don't appear be used.
    case 0b110:
    case 0b111:
    default:
        nowide::cerr << "compat: unknown case: " << (shuffle_flags & 0x3) << std::endl;
    }

    // BGP never changes from the 3rd offset
    bg_palettes[0] = compat_palettes[(*triplet)[2]];

    nowide::cout << "compat palletes chosen:" << std::endl;
    nowide::cout << "BGP: " << bg_palettes[0].to_rgb24_string() << std::endl;
    nowide::cout << "OBP0: " << obj_palettes[0].to_rgb24_string() << std::endl;
    nowide::cout << "OBP1: " << obj_palettes[1].to_rgb24_string() << std::endl;
}

PPU::obj_sprite_t PPU::oam_fetch_sprite(int index) {
    int loc = 0xFE00 + (index * 4);

    PPU::obj_sprite_t o;
    return o = {mem->read_oam(loc + 0, true),
                mem->read_oam(loc + 1, true),
                mem->read_oam(loc + 2, true),
                mem->read_oam(loc + 3, true)};
}

void PPU::enqueue_sprite_data(PPU::obj_sprite_t const &curr_sprite) {
    s8 pixel_line = y_cntr - (curr_sprite.pos_y - 16);
    u8 tile_num   = curr_sprite.tile_num & (LCDC_BIG_SPRITES ? ~1 : ~0);

    if(OBJ_Y_FLIP(curr_sprite)) {
        pixel_line = (LCDC_BIG_SPRITES ? 15 : 7) - pixel_line;
    }

    u16 addr = 0x8000 | (tile_num << 4) | (pixel_line << 1);

    bool bank1 = dev_is_GBC(device) && OBJ_GBC_VRAM_BANK(curr_sprite);

    u8 sprite_tile_1 = mem->read_vram(addr, true, bank1), sprite_tile_2 = mem->read_vram(addr + 1, true, bank1);

    u8 palette = !OBJ_GB_PALETTE(curr_sprite) ? reg(OBP0) : reg(OBP1);

    for(int i = 0; i < 8; i++) {
        u8 pix_idx = i;
        if(OBJ_X_FLIP(curr_sprite)) {
            pix_idx = 7 - pix_idx;
        }

        u8 tile_idx = ((sprite_tile_1 >> (7 - pix_idx)) & 1);
        tile_idx |= ((sprite_tile_2 >> (7 - pix_idx)) & 1) << 1;

        fifo_color_t color{};
        u8           palette_idx;
        if(dev_is_GBC(device)) {
            palette_idx     = OBJ_GBC_PALETTE(curr_sprite);
            color.color_idx = tile_idx;
        } else {
            tile_idx <<= 1;
            palette_idx     = 0;
            color.color_idx = static_cast<u8>((palette >> tile_idx) & 0x3_u8);
        }
        color.palette        = &obj_palettes[palette_idx];
        color.is_transparent = tile_idx == 0;
        color.priority       = !OBJ_PRIORITY(curr_sprite);

        if(sp_fifo->size() > i) {
            // lower idx sprites have priority, only replace pixels
            //  if one is transparent
            if(!color.is_transparent && sp_fifo->at(i).is_transparent) {
                sp_fifo->replace(i, color);
            }
        } else {
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
        if(active_sprites.size() < 10 && current_sprite.pos_x > 0 && y_cntr + 16 >= current_sprite.pos_y
           && y_cntr + 16 < current_sprite.pos_y + ((LCDC_BIG_SPRITES) ? 16 : 8)) {
            active_sprites.push_back(current_sprite);
        }

        sprite_counter++;
        oam_fetch_step = OAM_0;
        break;
    }

    // quit after scanning all 40 sprites
    if(sprite_counter == 40) {
        // reuse sprite_counter for rendering
        sprite_counter = 0;
        process_step   = SCANLINE_VRAM;

        // calculate background map address
        bg_map_addr =
                (0x9800) | ((LCDC_BG_TILE_MAP) ? 0x0400 : 0) | (((y_cntr + reg(SCY)) & 0xf8) << 2) | (reg(SCX) >> 3);
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
            attr_byte     = mem->read_vram(bg_map_addr, true, true); // read from bank 1
            bg_map_bank_1 = BG_VRAM_BANK(attr_byte);
        } else {
            bg_map_bank_1 = false;
        }

        // increment bg_map_addr, but only the bottom 5 bits.
        //  this will wrap around the edge of the tile map
        if(!skip_fetch) {
            bg_map_addr = (bg_map_addr & 0xFFE0) | ((bg_map_addr + 1) & 0x001F);
        }
        skip_fetch = false;

        vram_fetch_step = BM_1;
        break;

    // Background map clk 2
    case BM_1:
        tile_addr = 0x8000;
        if(Bit::test(bg_map_byte, 7)) {
            tile_addr += 0x0800;
        } else if(!LCDC_BG_WND_TILE_DATA) {
            tile_addr += 0x1000;
        }

        tile_y_line = ((y_cntr + reg(SCY)) & 0x7);
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
            attr_byte     = mem->read_vram(wnd_map_addr, true, true); // read from bank 1
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
        } else if(!LCDC_BG_WND_TILE_DATA) {
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
        vram_fetch_step = IDLE;
        break;

    // Idle Clocking
    case IDLE:
        // shift in bg pixels when the bg fifo is empty.
        // this will not occur on subsequent sprite reads as the bg_fifo
        // should be disabled until the sprite fetches are done

        // TODO: this should be moved out of the idle clocking as it makes
        // the first fetch 2 clock cycles too long
        if(bg_fifo->size() == 0) {
            for(int i = 0; i < 8; i++) {
                u8 x_pixel = ((dev_is_GBC(device) && BG_X_FLIP(attr_byte)) ? (i) : (7 - i));

                u8 tile_idx = ((tile_byte_1 >> x_pixel) & 1);
                tile_idx |= ((tile_byte_2 >> x_pixel) & 1) << 1;

                fifo_color_t color{};
                u8           palette_idx;
                if(dev_is_GBC(device)) {
                    color.priority  = BG_PRIORITY(attr_byte);
                    palette_idx     = BG_PALETTE(attr_byte);
                    color.color_idx = tile_idx & 0x3_u8;
                } else {
                    tile_idx <<= 1;
                    color.priority  = false;
                    palette_idx     = 0;
                    color.color_idx = (reg(BGP) >> tile_idx) & 0x3_u8;
                }

                color.palette        = &bg_palettes[palette_idx];
                color.is_transparent = false;

                bg_fifo->enqueue(color);
            }

            if(in_window) {
                vram_fetch_step = WM_0;
            } else {
                vram_fetch_step = BM_0;
            }
        }
        break;

    default:
        // invalid VRAM fetch step
        nowide::cerr << "vram step error" << vram_fetch_step << std::endl;
        assert(false);
    }

    /**
     * Actual PPU logic
     */
    if(bg_fifo->size() > 0) {
        fifo_color_t bg_color;
        bg_fifo->dequeue(bg_color);

        // if background is disabled, force write a 0
        if(!LCDC_BG_ENABLED && !dev_is_GBC(device)) {
            bg_color.color_idx = 0;
        }

        // if drawing a sprite, dequeue a sprite pixel
        if(sp_fifo->size() > 0) {
            fifo_color_t s_color;
            sp_fifo->dequeue(s_color);

            // this could all be a single if check but it would get messy as hell
            bool bg_has_priority     = dev_is_GBC(device) && bg_color.priority && LCDC_CGB_BG_PRIORITY;
            bool sprite_has_priority = s_color.priority && !bg_has_priority;
            bool should_draw_sprite  = bg_color.color_idx == 0 || sprite_has_priority;
            if(!s_color.is_transparent && should_draw_sprite) {
                bg_color = s_color;
            }
        }

        // skip first 8 pixels and first pixels of partially offscreen tiles
        if(x_cntr >= 8 + (reg(SCX) % 8)) {
            pix_clock_count++;

            // if frame is disabled, don't draw pixel data
            if(!frame_disable) {
                rgb15_to_rgb888(screen_buffer + current_byte, bg_color.palette->colors[bg_color.color_idx]);
                current_byte += 3;
            }
        }

        // if we finish the line, move to hblank and increment the window counter *if we're windowing*
        if(pix_clock_count == 160) {
            if(in_window) {
                wnd_y_cntr++;
            }
            process_step = HBLANK;
        }

        // rest of this cycle is prepping for next one
        x_cntr++;

        // Check Active Sprites to see if we should start displaying them
        if(LCDC_OBJ_ENABLED) {
            for(auto sprite : active_sprites) {
                if(x_cntr - (reg(SCX) % 8) == sprite.pos_x) {
                    pause_bg_fifo = true;
                    displayed_sprites.push_back(sprite);
                }
            }
        }

        // Check if we should switch to Window Rendering
        // I don't know where this +1 comes from... without it the window renders 1 pixel too early
        // TODO: just like...figure out why?
        if(!in_window && LCDC_WINDOW_ENABLED && x_cntr >= (reg(WX) + 1) && y_cntr >= reg(WY)) {
            in_window       = true;
            vram_fetch_step = WM_0;
            bg_fifo->clear();
            wnd_map_addr = 0x9800 | ((LCDC_WINDOW_TILE_MAP) ? 0x0400 : 0) | (wnd_y_cntr & 0xf8) << 2;
        }
    }
}

bool PPU::tick() {
    /**
     * TODO:
     *  - Looks like the GB Die has finally been extracted so eventually it would be wise
     * to decode the PPU logic into here
     *
     *  - How many of the PPU variables can we make static?
     */

    if(!LCDC_LCD_ENABLED) {
        // TODO: detail any further behavior?
        reg(LY) = 0;
        reg(STAT) &= ~STAT_MODE_FLAG; // clear mode bits

        new_frame   = true;
        first_frame = true;

        frame_clock_count = 0;

        return false;
    }

    if(Bit::risen(old_LCDC, reg(LCDC), LCDC_WINDOW_ENABLED_BIT)) {
        wnd_map_addr = 0x9800 | ((LCDC_WINDOW_TILE_MAP) ? 0x0400 : 0) | wnd_y_cntr << 2;
    }
    old_LCDC = reg(LCDC);

    /**
     * Occurs on Every Frame
     */
    if(new_frame) {
        new_frame = false;

        if(first_frame) {
            first_frame   = false;
            frame_disable = true;

            memset(screen_buffer, 0xFF, GB_S_P_SZ); // TODO: use the white color
        } else {
            frame_disable = false;
        }

        current_byte = 0;

        wnd_y_cntr = 0;
        // this is incremented to zero at the start of the first line
        y_cntr   = -1;
        new_line = true;

        vblank_int_requested = false;
    }

    /**
     * Occurs on Every line
     */
    if(new_line) {
        new_line   = false;
        skip_fetch = true;

        y_cntr++;

        x_cntr           = 0;
        pix_clock_count  = 0;
        line_clock_count = 0;

        sprite_counter = 0;
        active_sprites.clear();
        displayed_sprites.clear();

        bg_fifo->clear();
        pause_bg_fifo = false;

        in_window = false;

        reg(LY) = y_cntr; // set LY

        if(y_cntr < 144) {
            process_step    = SCANLINE_OAM;
            oam_fetch_step  = OAM_0;
            vram_fetch_step = BM_0;
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
        // frame disable is true for the first frame after the LCD is reenabled.

        /**
         * TODO: verify
         * Allegedly, or at least as far as BGB and gekkio's tests indicate,
         * The first frame after the LCD is reenabled, during the OAM state
         * the STAT mode flag is set to 0 instead of 2. It's corrected during
         * the transition to VRAM mode.
         **/
        reg(STAT) &= ~STAT_MODE_FLAG;
    } else {
        reg(STAT) = (reg(STAT) & ~STAT_MODE_FLAG) | process_step; // set current mode
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

    bool coin_int = Bit::test(reg(STAT), STAT_COIN_INT_BIT) && Bit::test(reg(STAT), STAT_COIN_BIT) && !coin_bit_signal
                 && old_LY != reg(LY);
    old_LY          = reg(LY);
    coin_bit_signal = Bit::test(reg(STAT), STAT_COIN_BIT);

    bool mode2_int  = (Bit::test(reg(STAT), STAT_MODE_2_INT_BIT) && process_step == SCANLINE_OAM),
         mode1_int  = (Bit::test(reg(STAT), STAT_MODE_1_INT_BIT) && process_step == VBLANK),
         mode0_int  = (Bit::test(reg(STAT), STAT_MODE_0_INT_BIT) && process_step == HBLANK),
         throw_stat = coin_int || (mode2_int && (mode2_int != old_mode2_int))
                   || (mode1_int && (mode1_int != old_mode1_int)) || (mode0_int && (mode0_int != old_mode0_int));
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
        // clock_count checker below will reset the frame

        if(line_clock_count >= 456) {
            // TODO: supposedly the first VBLANK is shorter than the rest? refer to TCAGBD and confirm
            new_line = true;
        }
    } else {
        nowide::cerr << "process_step error: " << as_hex((int)process_step) << std::endl;
    }

    line_clock_count++; // used in HBLANK and VBLANK
    frame_clock_count++;

    if(frame_clock_count < TICKS_PER_FRAME) {
        return false;
    } else {
        frame_clock_count = 0;
        new_frame         = true;
        return true;
    }
}