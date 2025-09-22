#pragma once

#include "util/file.hpp"
#include "util/types/pixel.hpp"
#include "util/types/ringbuffer.hpp"

#include "cpu.hpp"
#include "defs.hpp"
#include "io.hpp"
#include "ppu.hpp"

struct breakpoint_exception {
    breakpoint_exception() = default;
};

namespace Silver {

    class Core {
    public:
        static constexpr u32 native_width       = PPU::native_width;
        static constexpr u32 native_height      = PPU::native_height;
        static constexpr u32 native_pixel_count = PPU::native_pixel_count;

        explicit Core(
                const std::shared_ptr<Silver::File>                &rom,
                const std::optional<std::shared_ptr<Silver::File>> &bootrom = std::nullopt,
                gb_device_t                                         device  = device_GBC);
        ~Core();

        // void                              init_thread(bool paused = true);
        // void                              pause_thread();
        // bool                              thread_paused();
        // void                              resume_thread();
        // void                              stop_thread();
        // void                              run_thread();

        void                              tick_once();
        void                              tick_instr();
        void                              tick_frame();
        void                              tick_delta_or_frame();
        //    void tick_audio_buffer(u8* buf, int buf_len);

        bool                              is_frame_ready();

        void                              set_input_state(Joypad::button_states_t const &state);
        void                              do_audio_callback(float *buff, int copy_cnt);
        const std::vector<Silver::Pixel> &getPixelBuffer();

        CPU::registers_t                  getRegistersFromCPU();
        Memory::io_registers_t            getregistersfromIO();
        u8                                getByteFromIO(u16 addr);

        std::pair<u8, u8>                 getTileLineByAddr(u16 addr, bool bank1);
        void               parseTileLine(std::array<Silver::Pixel, 8> &arr, u8 byte_1, u8 byte_2, u8 bg_attr);
        std::pair<u16, u8> calcTileAddrForCoordinate(bool window, u8 x, u8 y);

        std::vector<u8>    getOAMEntry(int index);
        void               getVRAMBuffer(std::vector<Pixel> &vec, u8 vramIdx, bool vramBank);
        void               getBGBuffer(std::vector<Pixel> &vec);
        // void getWNDBuffer(u8 *buf);

        void               set_bp(u16 bp, bool en = false);
        u16                get_bp();
        void               set_bp_active(bool en);
        bool               get_bp_active();

    private:
        Memory              *mem;
        Cartridge           *cart;
        APU                 *apu;
        PPU                 *ppu;
        Joypad              *joy;
        IO_Bus              *io;
        CPU                 *cpu;

        gb_device_t          device;

        static constexpr u32 audio_buffer_sz            = 2048;
        using AudioBuffer                               = std::array<float, audio_buffer_sz>;

        bool                                frame_ready = false;
        jnk0le::Ringbuffer<AudioBuffer, 4> *audio_queue = nullptr;
        std::vector<float>                  audio_vector;

        u16                                 breakpoint = 0;
        bool                                bp_active  = false;
    };

} // namespace Silver