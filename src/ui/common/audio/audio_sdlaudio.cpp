#include "audio.hpp"

#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL_audio.h>

#include "gb_core/core.hpp"

#include "util/log.hpp"

SDL_AudioSpec desired = {
    .format   = SDL_AUDIO_F32,
    .channels = CHANNEL_CNT,
    .freq     = SAMPLE_RATE,
};

struct SDLAudioManagerContext {
    SDL_AudioDeviceID audio_device;
    SDL_AudioStream  *audio_stream;
};

std::vector<u8> stream_buf;
extern "C" void _audio_callback(void *userdata, SDL_AudioStream *stream, int additional, int total) {
    auto core = static_cast<Silver::Core *>(userdata);

    if(stream_buf.max_size() < additional * 4) {
        stream_buf.reserve(additional * 4);
    }

    core->do_audio_callback((float *)stream_buf.data(), additional);

    auto succeeded = SDL_PutAudioStreamData(stream, stream_buf.data(), additional);
    if(!succeeded) {
        LogError("AudioManager") << "PutAudioStreamData failed: " << SDL_GetError();
    }
}

Silver::AudioManager::AudioManager() :
    audio_dev(nullptr) { }

Silver::AudioManager *Silver::AudioManager::init_audio(std::shared_ptr<Silver::Core> core) {
    auto audio_manager            = new AudioManager();
    audio_manager->core           = std::move(core);
    audio_manager->audio_dev      = new SDLAudioManagerContext {0, nullptr};

    SDL_AudioStream *audio_stream = SDL_CreateAudioStream(&desired, &desired);
    if(audio_stream == nullptr) {
        LogError("AudioManager/SDLAudio") << "Failed to open audio stream: " << SDL_GetError();
        return nullptr;
    }

    SDL_SetAudioStreamGetCallback(audio_stream, _audio_callback, static_cast<void *>(core.get()));

    static_cast<SDLAudioManagerContext *>(audio_manager->audio_dev)->audio_stream = audio_stream;

    // TOOD: if theres an audio_device selected in the config, pass it here
    audio_manager->set_audio_device(std::nullopt);

    return audio_manager;
}

Silver::AudioManager::~AudioManager() {
    SDL_CloseAudioDevice(static_cast<SDLAudioManagerContext *>(this->audio_dev)->audio_device);
    SDL_DestroyAudioStream(static_cast<SDLAudioManagerContext *>(this->audio_dev)->audio_stream);
    delete static_cast<SDLAudioManagerContext *>(this->audio_dev);
}

void Silver::AudioManager::set_audio_device(std::optional<Silver::AudioDevice> const &maybe_dev) {
    SDL_CloseAudioDevice(static_cast<SDLAudioManagerContext *>(this->audio_dev)->audio_device);

    SDL_AudioDeviceID dev_id = maybe_dev.has_value()
                                     ? SDL_OpenAudioDevice((SDL_AudioDeviceID)(uintptr_t)maybe_dev->id, &desired)
                                     : SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
    if(dev_id == 0) {
        LogError("AudioManager/SDLAudio") << "Failed to open audio device: " << SDL_GetError();
        return;
    }

    SDL_BindAudioStream(dev_id, static_cast<SDLAudioManagerContext *>(this->audio_dev)->audio_stream);

    static_cast<SDLAudioManagerContext *>(this->audio_dev)->audio_device = dev_id;
}

std::vector<Silver::AudioDevice> Silver::AudioManager::get_audio_devices() {
    std::vector<Silver::AudioDevice> devices;

    int                              num_devices = 0;
    auto                             devices_ptr = SDL_GetAudioPlaybackDevices(&num_devices);
    if(devices_ptr == nullptr) {
        LogError("AudioManager/SDLAudio") << "Failed to get audio devices: " << SDL_GetError();
        return devices;
    }

    if(num_devices <= 0) {
        LogError("AudioManager/SDLAudio") << "No audio devices found.";
        return devices;
    }

    for(int i = 0; i < num_devices; i++) {
        Silver::AudioDevice device;
        device.id   = (void *)(uintptr_t)devices_ptr[i];
        device.name = SDL_GetAudioDeviceName(devices_ptr[i]);
        devices.push_back(device);
    }

    return devices;
}

void Silver::AudioManager::start_audio() {
    SDL_ResumeAudioDevice(static_cast<SDLAudioManagerContext *>(this->audio_dev)->audio_device);
}

void Silver::AudioManager::stop_audio() {
    SDL_PauseAudioDevice(static_cast<SDLAudioManagerContext *>(this->audio_dev)->audio_device);
}
