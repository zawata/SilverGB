#include "rtaudio/RtAudio.h"

#include "audio.hpp"

int _audio_callback(
    void *outputBuffer,
    void *inputBuffer,
    unsigned int nBufferFrames,
    double streamTime,
    RtAudioStreamStatus status,
    void *userData
) {
    //NOTE: nBufferFrames is equal to
    // buff_size / channel_count / sizeof(float)
    static_cast<GB_Core *>(userData)->do_audio_callback((float *)outputBuffer, nBufferFrames);
    return 0;
}

GB_Audio::GB_Audio() {}

GB_Audio *GB_Audio::init_audio(GB_Core *core) {
    auto gb_audio = new GB_Audio();
    RtAudio *audio_dev;

    try {
	    gb_audio->audio_dev = audio_dev = new RtAudio();
    }
    catch(RtAudioError e){
	    std::cerr << "fail to create RtAudio: " << e.what() << std::endl;
	    return nullptr;
    }

    if (!audio_dev){
	    std::cerr << "fail to allocate RtAudio" << std::endl;
	return nullptr;
    }
    /* probe audio devices */
    unsigned int devId = audio_dev->getDefaultOutputDevice();

    /* Setup output stream parameters */
    RtAudio::StreamParameters *outParam = new RtAudio::StreamParameters();

    outParam->deviceId = devId;
    outParam->nChannels = CHANNEL_CNT;

    u32 frame_count = SAMPLE_CNT;

    audio_dev->openStream(
            outParam,        // outputParameters
            NULL,            // inputParameters
            RTAUDIO_FLOAT32, // format
            SAMPLE_RATE,     // sampleRate
            &frame_count,    // bufferFrames
            _audio_callback, // callback
            core);          // userData

    std::cout << "frame_count: " << frame_count << std::endl;

    return gb_audio;
}

GB_Audio::~GB_Audio() {
    delete static_cast<RtAudio *>(this->audio_dev);
}


void GB_Audio::start_audio() {
    static_cast<RtAudio *>(this->audio_dev)->startStream();
}

void GB_Audio::stop_audio() {
    static_cast<RtAudio *>(this->audio_dev)->stopStream();
}