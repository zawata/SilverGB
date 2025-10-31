#include "audio.hpp"

#include <cstdint>
#include <memory>
#include <utility>

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

Silver::AudioManager::AudioManager() :
    audio_dev(), core() { }

Silver::AudioManager *Silver::AudioManager::init_audio(std::shared_ptr<Silver::Core> core) {
    PaError err = Pa_Initialize();
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Initializing: " << Pa_GetErrorText(err);
        return nullptr;
    }

    auto audio_manager       = new AudioManager();
    audio_manager->core      = std::move(core);
    audio_manager->audio_dev = nullptr;
    audio_manager->create_stream(std::nullopt);

    return audio_manager;
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

void Silver::AudioManager::create_stream(std::optional<Silver::AudioDevice> const &device) {
    if(this->audio_dev != nullptr) {
        LogError("AudioManager/PortAudio") << "Stream already created!";
        return;
    }

    PaDeviceIndex deviceIndex = device.has_value() ? (PaDeviceIndex)(intptr_t)device->id : Pa_GetDefaultOutputDevice();

    PaStreamParameters outputParameters = {
        .device           = deviceIndex,
        .channelCount     = CHANNEL_CNT,
        .sampleFormat     = paFloat32,
        .suggestedLatency = Pa_GetDeviceInfo(deviceIndex)->defaultLowOutputLatency,
    };

    PaError err = Pa_OpenStream(
            static_cast<PaStream **>(&this->audio_dev), // stream Device
            nullptr,                                    // input stream paramters
            &outputParameters,                          // output parameters
            SAMPLE_RATE,                                // sampleRate
            SAMPLE_CNT,                                 // TODO change this to paFramesPerBufferUnspecified
            paNoFlag,                                   // stream flags
            _audio_callback,                            // callback
            this->core.get()                            // TODO: is this safe?
    );

    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error createStream: " << Pa_GetErrorText(err);
        return;
    }
}

void Silver::AudioManager::close_stream() {
    PaError err = Pa_CloseStream(static_cast<PaStream *>(this->audio_dev));
    if(err != paNoError) {
        LogError("AudioManager/PortAudio") << "Error Closing Stream: " << Pa_GetErrorText(err);
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

std::vector<Silver::AudioDevice> Silver::AudioManager::get_audio_devices() {
    std::vector<Silver::AudioDevice> devices;

    int                              deviceCount = Pa_GetDeviceCount();
    if(deviceCount < 0) {
        LogError("AudioManager/PortAudio") << "Error Getting Device Count: " << Pa_GetErrorText(deviceCount);
        return devices;
    }

    for(int i = 0; i < deviceCount; i++) {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
        if(deviceInfo != nullptr) {
            Silver::AudioDevice device;
            device.id   = (void *)(uintptr_t)i;
            device.name = deviceInfo->name;
            devices.push_back(device);
        }
    }

    return devices;
}

void set_audio_device(Silver::AudioDevice device) { PaDeviceIndex deviceIndex = (PaDeviceIndex)(uintptr_t)device.id; }
