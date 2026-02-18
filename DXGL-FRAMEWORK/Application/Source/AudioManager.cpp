
#include "AudioManager.h"
#include "Utils.h"




void AudioManager::SetDirectoryMUS(const std::string& directoryPath) {
    directoryMusic = directoryPath;

    if (directoryMusic.back() != '/') {
        directoryMusic += "/";
    }
}

void AudioManager::SetDirectorySFX(const std::string& directoryPath) {
    directorySFX = directoryPath;

    if (directorySFX.back() != '/') {
        directorySFX += "/";
    }
}

void AudioManager::InitSystem() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
        SDL_Log("initSystem: SDL_Init Error: %s", SDL_GetError());
}

void AudioManager::OpenMixer() {
    int flags = MIX_INIT_OGG | MIX_INIT_MP3; // init flag to support ogg and mp3 files
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags)  // initted is a bitmask, this check if the flags are inited correctly
        SDL_Log("openMixer: Mix_Init failed to init required formats: %s", Mix_GetError());

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        SDL_Log("openMixer: Mix_OpenAudio Error: %s", Mix_GetError());

    Mix_AllocateChannels(TOTAL_SFX_CHANNEL);
}

void AudioManager::LoadSFX(unsigned key, const char* filename) {
    sfxList[key] = Mix_LoadWAV((directorySFX + filename).c_str());
    if (!sfxList[key])
        SDL_Log("loadSFX: Mix_LoadWAV Error: %s", Mix_GetError());
}

void AudioManager::LoadMUS(const char* filename, double durationInSeconds) {
    music = Mix_LoadMUS((directoryMusic + filename).c_str());
    if (music == NULL)
        music = nullptr;
    if (!music)
        SDL_Log("loadMUS: Mix_LoadMUS Error: %s", Mix_GetError());
    else
        musicDuration = durationInSeconds;
}

void AudioManager::UnloadSFX(unsigned key) {
    if (sfxList[key]) {
        Mix_FreeChunk(sfxList[key]);
        sfxList.erase(key);
    }
}

void AudioManager::UnloadSFXAll() {
    for (auto& pair : sfxList)
        if (pair.second)
            Mix_FreeChunk(pair.second);
    sfxList.clear();
}

void AudioManager::UnloadMUS() {
    if (music) {
        if (PlayingMUS()) {
            PauseMUS();
        }
        Mix_FreeMusic(music);
        music = nullptr;
    }
}

void AudioManager::UnloadAll() {
    UnloadSFXAll();
    UnloadMUS();
}

// | channel : -1 = find first free channel | loops : -1 = loop forever
void AudioManager::PlaySFX(unsigned key, int channel, int loops) {
    auto it = sfxList.find(key);
    if (it != sfxList.end() && it->second)
        Mix_PlayChannel(channel, sfxList[key], loops);
}

// | loops : -1 = loop forever | type : 0 = no fade, 1 = fade in
void AudioManager::PlayMUS(int loops, unsigned type, int duration_ms) {
    if (music) {
        if (type)
            Mix_FadeInMusic(music, loops, duration_ms);
        else
            Mix_PlayMusic(music, loops);
        musicPlaying = true;
    }
}

// channel : -1 = set all channels | volume : range from 0 - 1, -1 to get current volume
float AudioManager::VolumeChannel(int channel, float volume) {
    int volume_int = static_cast<int>(volume * 128);
    if (volume < 0)
        volume_int = -1;

    return Mix_Volume(channel, volume_int) / 128.f;
}

// | volume : range from 0 - 1, -1 to get current volume
float AudioManager::VolumeSFX(unsigned key, float volume) {
    int volume_int = static_cast<int>(volume * 128);
    if (volume < 0)
        volume_int = -1;

    auto it = sfxList.find(key);
    if (it != sfxList.end() && it->second)
        return Mix_VolumeChunk(sfxList[key], volume_int) / 128.f;
    return -1;
}

// | volume : range from 0 - 1, -1 to get current volume
float AudioManager::VolumeMUS(float volume) {
    int volume_int = static_cast<int>(volume * 128);
    if (volume < 0)
        volume_int = -1;

    if (music)
        return Mix_VolumeMusic(volume_int) / 128.f;
    return -1;
}

// channel : -1 = stop all channels
void AudioManager::StopChannel(int channel) {
    Mix_HaltChannel(channel);
}

// return playback position in seconds
double AudioManager::GetMUSPosition() {
    return Mix_GetMusicPosition(music);
}

void AudioManager::SetMUSPosition(double postionInSeconds) {
    postionInSeconds = Clamp(postionInSeconds, 0, musicDuration);
    Mix_SetMusicPosition(postionInSeconds);
}

double AudioManager::GetMUSDUration() {
    return musicDuration;
}

int AudioManager::PlayingMUS() {
    return musicPlaying;
}

int AudioManager::PlayingSFX(int channel) {
    return Mix_Playing(channel);
}

void AudioManager::PauseMUS() {
    Mix_PauseMusic();
    musicPlaying = false;
}

void AudioManager::ResumeMUS() {
    Mix_ResumeMusic();
    musicPlaying = true;
}

void AudioManager::RewindMUS() {
    Mix_RewindMusic();
    musicPlaying = true;
}

void AudioManager::FadeOutMUS(int duration_ms) {
    Mix_FadeOutMusic(duration_ms);
}

void AudioManager::CloseMixer() {
    UnloadAll();
    Mix_CloseAudio();
    sfxList.clear();
}

void AudioManager::ExitSystem() {
    Mix_Quit();
    SDL_Quit();
}

AudioManager::~AudioManager() {}

