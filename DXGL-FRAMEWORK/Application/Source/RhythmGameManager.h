
#ifndef RHYTHM_GAME_MANAGER_H
#define RHYTHM_GAME_MANAGER_H

#include <glm\glm.hpp>

#include <string>
#include <array>
#include <vector>
#include <memory>

#include "Event.h"

class RenderObject;


class RhythmBeat {
public:
    int beat;
    std::weak_ptr<RenderObject> render;

    static Event<void, RhythmBeat*> createEvent;

    static RhythmBeat* MakeBeat(int beat = 0) {
        RhythmBeat* rb = new RhythmBeat(beat);
        createEvent.Invoke(rb);
        return rb;
    }

    RhythmBeat(int beat = 0)
        : beat(beat) {}
};


class RhythmNote {
public:

    enum NOTE_TYPE {
        TAP,
        HOLD,

        INVALID
    };

    NOTE_TYPE type = INVALID;
    int lane;
    float beat;

    static Event<void, RhythmNote*> activeEvent;
    static Event<void, RhythmNote*> hitEvent;

    virtual ~RhythmNote() = 0;
};

class TapNote : public RhythmNote {
public:
    std::weak_ptr<RenderObject> render;

    TapNote() {
        type = TAP;
    }
    ~TapNote() override {}
};

class HoldNote : public RhythmNote {
public:
    float endBeat;
    std::weak_ptr<RenderObject> startRender;
    std::weak_ptr<RenderObject> lengthRender;
    std::weak_ptr<RenderObject> endRender;

    bool holding = false;
    float holdTimer = 0;
    float holdTimerScore = 0;

    HoldNote(float endBeat = -1) {
        type = HOLD;

        if (endBeat == -1)
            this->endBeat = beat;
        else
            this->endBeat = endBeat;
    }
    ~HoldNote() override {}
};


class RhythmLane {
public:
    glm::vec3 position;
    glm::vec3 direction;
    float length;
    float startFraction;
    float endFraction;
    std::vector<RhythmNote*> activeNotes;
    std::vector<RhythmBeat*> displayBeats;

    void SetLane(glm::vec3 position, glm::vec3 direction, float length, float startFraction, float endFraction);

    void CleanUp();

    ~RhythmLane() {
        CleanUp();
    }
};


class RhythmChart {
public:
    std::string musicFilePath;
    float musicDuration;

    float BPM;
    float ScrollSpeed;
    std::vector<RhythmNote*> notes;
    float endTime;
    float offsetStartBeat;
    float musicOffsetBeat;

    void CleanUp();

    ~RhythmChart() {
        CleanUp();
    }
};


class RhythmGameManager {
public:

    struct Progress {

        enum PROGRESSION_TYPE {
            SUCCESS,
            PACKET_LOSS
        };

        float amount;
        PROGRESSION_TYPE type;

        Progress(float amount, PROGRESSION_TYPE type)
            : amount(amount), type(type) {}
    };

    static RhythmGameManager& GetInstance() {
        static RhythmGameManager rhythmGameManager;
        return rhythmGameManager;
    }

    enum SCORE_TYPE {
        MISS,
        GOOD,
        GREAT,
        PERFECT,

        TOTAL_SCORE_TYPE
    };

    void SetDirectory(const std::string& directoryPath);

    void SetDetectRange(const std::array<float, TOTAL_SCORE_TYPE>& detectRange) {
        this->detectRange = detectRange;
    }

    void Update(double dt);

    void LoadGame(const std::string& chartFilePath);
    void StartGame();
    void EndGame();
    bool CheckMusicPlaying() {
        return musicPlaying;
    }
    bool CheckGameActive() {
        return active;
    }

    SCORE_TYPE GetScoreType() {
        return lastestScore;
    }
    int GetScore() {
        return score;
    }
    std::array<RhythmLane, 4>& GetLanes() {
        return lanes;
    }

    void SetTappedLane(int laneIndex, bool tapped) {
        tappedLane[laneIndex] = tapped;
    }
    void SetHeldLane(int laneIndex, bool held) {
        heldLane[laneIndex] = held;
    }
    bool GetTappedLane(int laneIndex) {
        return tappedLane[laneIndex];
    }
    bool GetHeldLane(int laneIndex) {
        return heldLane[laneIndex];
    }

    void SetAutoPlay(bool isAutoPlay) {
        autoPlay = isAutoPlay;
    }

    float GetCurrentBeat() {
        return currentBeat;
    }
    float GetMaxDisplayBeat() {
        return maxDisplayBeat;
    }
    const std::vector<Progress>& GetProgression() {
        return progression;
    }
    int GetMaxProgression() {
        return maxProgressionCount;
    }
    bool GetProgressionMaxed() {
        return progressionMaxed;
    }
    bool GetProgressionFixed() {
        return progressionFixed;
    }

    void SetTickSFXKey(unsigned int key) {
        tickSFXKey = key;
    }

private:

    void LoadChart(const std::string& filePath);

    float ScoreChart(SCORE_TYPE type);
    bool CheckHitNote(float noteBeat);
    void AddScore(SCORE_TYPE type);
    int HeldScore(float heldDurationInBeats);
    void UpdateProgress(Progress::PROGRESSION_TYPE type);

    std::string directory;

    unsigned int tickSFXKey;
    bool active = false;
    bool musicPlaying = false;
    RhythmChart chart;
    std::array<float, TOTAL_SCORE_TYPE> detectRange;
    std::array<RhythmLane, 4> lanes;
    std::array<bool, 4> tappedLane;
    std::array<bool, 4> heldLane;

    std::vector<RhythmNote*> notesLeft;
    float maxDisplayBeat;
    int prevMaxDisplayBeat_int;
    float currentBeat;
    int prevCurrentBeat_int = 0;
    float BPS;
    float gameElapsed;

    std::vector<Progress> progression;
    int progressionCount;
    int maxProgressionCount;
    float requiredProgressionPercentage = 0.8f;
    bool progressionMaxed = false;
    bool progressionFixed = true;

    SCORE_TYPE lastestScore;
    int score;
    float heldScoreTime = 0.1f;
    int heldScoring = 5;

    bool autoPlay = false;

    RhythmGameManager() = default;
    ~RhythmGameManager() = default;
    RhythmGameManager(const RhythmGameManager&) = delete;
    RhythmGameManager& operator=(const RhythmGameManager&) = delete;
};


#endif
