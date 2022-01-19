#if defined(PA_ENABLE_DEBUG_OUTPUT)
    #undef PA_ENABLE_DEBUG_OUTPUT
#endif
#include "portaudio.h"

#include "audio.hpp"

int _audio_callback(
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long nBufferFrames,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
) {
    //NOTE: nBufferFrames is equal to
    // buff_size / channel_count / sizeof(float)
    static_cast<Silver::Core *>(userData)->do_audio_callback((float *)outputBuffer, nBufferFrames);
    return 0;
}

GB_Audio::GB_Audio(void *audio_dev): audio_dev(audio_dev) {}

GB_Audio *GB_Audio::init_audio(Silver::Core *core) {
    PaStream *stream;

    PaError err = Pa_Initialize();
    if(err != paNoError) {
        nowide::cerr << "Error Initializing: " << Pa_GetErrorText(err) << std::endl;
        return nullptr;
    }

    //TODO: input/output parameters with Pa_OpenStream
    err = Pa_OpenDefaultStream(
            &stream,          // stream Device
            0,          /* no input channels */
            CHANNEL_CNT,          /* stereo output */
            paFloat32,  /* 32 bit floating point output */
            SAMPLE_RATE,     // sampleRate
            SAMPLE_CNT,    // TODO change this to paFramesPerBufferUnspecified
            _audio_callback, // callback
            core);          // userData
    if(err != paNoError) {
        nowide::cerr << "Error Initializing: " << Pa_GetErrorText(err) << std::endl;
        return nullptr;
    }

    return new GB_Audio(stream);
}

GB_Audio::~GB_Audio() {
    PaError err = Pa_CloseStream(static_cast<PaStream *>(this->audio_dev));
    if(err != paNoError) {
        nowide::cerr << "Error Closing Stream: " << Pa_GetErrorText(err) << std::endl;
    }

    err = Pa_Terminate();
    if(err != paNoError) {
        nowide::cerr << "Error Terminating: " << Pa_GetErrorText(err) << std::endl;
    }
}


void GB_Audio::start_audio() {
    PaError err = Pa_StartStream(static_cast<PaStream *>(this->audio_dev));
    if(err != paNoError) {
        nowide::cerr << "Error Starting Stream: " << Pa_GetErrorText(err) << std::endl;
    }
}

void GB_Audio::stop_audio() {
    PaError err = Pa_StopStream(static_cast<PaStream *>(this->audio_dev));
    if(err != paNoError) {
        nowide::cerr << "Error Stopping Stream: " << Pa_GetErrorText(err) << std::endl;
    }
}