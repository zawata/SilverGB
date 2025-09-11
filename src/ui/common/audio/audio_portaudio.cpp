#include "audio.hpp"

#include "util/log.hpp"

#include "portaudio.h"

int _audio_callback(
        const void *inputBuffer, void *outputBuffer, unsigned long nBufferFrames,
        const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    // NOTE: nBufferFrames is equal to
    //  buff_size / channel_count / sizeof(float)
    static_cast<Silver::Core *>(userData)->do_audio_callback((float *)outputBuffer, nBufferFrames);
    return 0;
}

Silver::AudioManager::AudioManager(void *audio_dev) :
    audio_dev(audio_dev) { }

Silver::AudioManager *Silver::AudioManager::init_audio(Silver::Core *core) {
    PaStream *stream;

    PaError   err = Pa_Initialize();
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Initializing: " << Pa_GetErrorText(err);
        return nullptr;
    }

    // TODO: input/output parameters with Pa_OpenStream
    err = Pa_OpenDefaultStream(
            &stream,         // stream Device
            0,               // no input channels
            CHANNEL_CNT,     // stereo output
            paFloat32,       // 32 bit floating point output
            SAMPLE_RATE,     // sampleRate
            SAMPLE_CNT,      // TODO change this to paFramesPerBufferUnspecified
            _audio_callback, // callback
            core             // userData
    );
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Initializing: " << Pa_GetErrorText(err);
        return nullptr;
    }

    return new AudioManager(stream);
}

Silver::AudioManager::~AudioManager() {
    PaError err = Pa_CloseStream(static_cast<PaStream *>(this->audio_dev));
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Closing Stream: " << Pa_GetErrorText(err);
    }

    err = Pa_Terminate();
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Terminating: " << Pa_GetErrorText(err);
    }
}

void Silver::AudioManager::start_audio() {
    PaError err = Pa_StartStream(static_cast<PaStream *>(this->audio_dev));
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Starting Stream: " << Pa_GetErrorText(err);
    }
}

void Silver::AudioManager::stop_audio() {
    PaError err = Pa_StopStream(static_cast<PaStream *>(this->audio_dev));
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Stopping Stream: " << Pa_GetErrorText(err);
    }
}