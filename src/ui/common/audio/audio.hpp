#include "gb_core/core.hpp"

#define SAMPLE_RATE 48000
#define CHANNEL_CNT 1
#define SAMPLE_CNT  2048


struct GB_Audio {
    ~GB_Audio();

    static GB_Audio *init_audio(GB_Core *core);

    void start_audio();
    void stop_audio();
private:
    GB_Audio();

    void *audio_dev;
};