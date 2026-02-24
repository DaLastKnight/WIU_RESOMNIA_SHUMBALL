
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

    ~RhythmLane() {
        for (auto& beat : displayBeats) {
            delete beat;
        }
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

    ~RhythmChart() {
        for (auto& note : notes) {
            delete note;
        }
    }
};


class RhythmGameManager {
public:

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
    void StartGame() {
        active = true;
    }
    bool CheckGameActive() {
        return active;
    }

    SCORE_TYPE GetScoreType() {
        return lastestScore;
    }
    std::array<RhythmLane, 4>& GetLanes() {
        return lanes;
    }

    void TappedLane(int laneIndex) {
        tappedLane[laneIndex] = true;
    }
    void HeldLane(int laneIndex) {
        heldLane[laneIndex] = true;
    }
    bool GetTappedLane(int laneIndex) {
        return tappedLane[laneIndex] = true;
    }
    bool GetHeldLane(int laneIndex) {
        return heldLane[laneIndex] = true;
    }

    float maxDisplayBeat;
    int prevMaxDisplayBeat_int;
    float currentBeat;

private:

    void LoadChart(const std::string& filePath);

    bool CheckHitNote(float noteBeat);

    std::string directory;

    bool active = false;
    RhythmChart chart;
    std::array<float, TOTAL_SCORE_TYPE> detectRange;
    std::array<RhythmLane, 4> lanes;
    std::array<bool, 4> tappedLane;
    std::array<bool, 4> heldLane;

    std::vector<RhythmNote*> notesLeft;
   
    float BPS;
    float gameElapsed;
    SCORE_TYPE lastestScore;

    RhythmGameManager() = default;
    ~RhythmGameManager() = default;
    RhythmGameManager(const RhythmGameManager&) = delete;
    RhythmGameManager& operator=(const RhythmGameManager&) = delete;
};


#endif
