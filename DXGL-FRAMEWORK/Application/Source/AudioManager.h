// JH
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL.h>
#include <SDL_mixer.h>

#include <map>
#include <string>


enum class SFX_TYPE : int;

class AudioManager {
public:

    static AudioManager& GetInstance() {
        static AudioManager audioManager;
        return audioManager;
    }

    void SetDirectoryMUS(const std::string& directoryPath);
    void SetDirectorySFX(const std::string& directoryPath);

    void InitSystem();
    void OpenMixer();

    void LoadSFX(unsigned key, const char* filename);
    void LoadMUS(const char* filename, double totalMusicDuration);

    void UnloadSFX(unsigned key);
    void UnloadSFXAll();
    void UnloadMUS();
    void UnloadAll();

    // | channel : -1 = find first free channel | loops : -1 = loop forever
    void PlaySFX(unsigned key, int channel = -1, int loops = 0);
    // | loops : -1 = loop forever | type : 0 = no fade, 1 = fade in
    void PlayMUS(int loops = 0, unsigned type = 0, int duration_ms = 0);

    // channel : -1 = set all channels | volume : range from 0 - 1, -1 to get current volume
    float VolumeChannel(int channel, float volume);
    // | volume : range from 0 - 1, -1 to get current volume
    float VolumeSFX(unsigned key, float volume);
    // | volume : range from 0 - 1, -1 to get current volume
    float VolumeMUS(float volume);

    // channel : -1 = stop all channels
    void StopChannel(int channel = -1);

    // return playback position in seconds
    double GetMUSPosition();
    void SetMUSPosition(double postionInSeconds);
    double GetMUSDUration();

    int PlayingMUS();
    // | channel : -1 = return number of channels currently playing
    int PlayingSFX(int channel = -1);

    void PauseMUS();
    void ResumeMUS();
    void RewindMUS();
    void FadeOutMUS(int duration_ms);

    void CloseMixer();
    void ExitSystem();

private:

    static constexpr unsigned TOTAL_SFX_CHANNEL = 32;
    std::map<int, Mix_Chunk*> sfxList;
    Mix_Music* music = nullptr;
    double musicDuration;
    bool musicPlaying = false;

    std::string directoryMusic;
    std::string directorySFX;

    AudioManager() = default;
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
};

#endif