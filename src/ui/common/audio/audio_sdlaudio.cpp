#include "audio.hpp"

#include <SDL2/SDL.h>

extern "C"
void _audio_callback(void* userdata, uint8_t* stream, int len) {
    static_cast<Silver::Core *>(userdata)->do_audio_callback((float *)stream, len / 4);
}

GB_Audio::GB_Audio(void *audio_dev): audio_dev(audio_dev) {}

GB_Audio *GB_Audio::init_audio(Silver::Core *core) {

    SDL_AudioSpec desired = { 0 };
    SDL_AudioDeviceID *audio_dev = new SDL_AudioDeviceID;

    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_F32;
    desired.channels = CHANNEL_CNT;
    desired.samples = SAMPLE_CNT;
    desired.userdata = core;
    desired.callback = _audio_callback;

    *audio_dev = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if(!*audio_dev) {
        nowide::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return new GB_Audio(audio_dev);
}

GB_Audio::~GB_Audio() {
    SDL_CloseAudioDevice(*(SDL_AudioDeviceID *)audio_dev);
    SDL_CloseAudio();
    delete static_cast<SDL_AudioDeviceID *>(this->audio_dev);
}


void GB_Audio::start_audio() {
    SDL_PauseAudioDevice(*static_cast<SDL_AudioDeviceID *>(this->audio_dev), 0);
}

void GB_Audio::stop_audio() {
    SDL_PauseAudioDevice(*static_cast<SDL_AudioDeviceID *>(this->audio_dev), 1);
}