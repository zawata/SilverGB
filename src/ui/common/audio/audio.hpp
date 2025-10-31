#pragma once

#include <memory>

#include "gb_core/core.hpp"

#define SAMPLE_RATE 48000
#define CHANNEL_CNT 1
#define SAMPLE_CNT  2048

namespace Silver {

    struct AudioDevice {
        void       *id;
        std::string name;
    };

    struct AudioManager {
        ~AudioManager();

        static AudioManager     *init_audio(std::shared_ptr<Silver::Core> core);

        void                     start_audio();
        void                     stop_audio();

        std::vector<AudioDevice> get_audio_devices();
        void                     set_audio_device(std::optional<Silver::AudioDevice> const &maybe_dev);

    private:
        explicit AudioManager();

        std::shared_ptr<Silver::Core> core;
        void                         *audio_dev;
    };

} // namespace Silver
