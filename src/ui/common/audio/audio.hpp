#pragma once

#include "gb_core/core.hpp"

#define SAMPLE_RATE 48000
#define CHANNEL_CNT 1
#define SAMPLE_CNT  2048

namespace Silver {

    struct AudioManager {
        ~AudioManager();

        static AudioManager *init_audio(Silver::Core *core);

        void                 start_audio();
        void                 stop_audio();

    private:
        AudioManager(void *audio_dev);

        void *audio_dev;
    };

} // namespace Silver