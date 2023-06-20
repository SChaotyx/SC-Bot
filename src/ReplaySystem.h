#pragma once

#include "includes.h"
#include "Replay.h"
#include "PracticeFix.h"
#include "Recorder.h"

enum RSState {
    NOTHING,
    RECORDING,
    PLAYING
};

class ReplaySystem {
    Replay replay;
    RSState state = NOTHING;
    int defFps;

    int frameOffset = 0;
    size_t actionIndex = 0;

    PracticeFix practiceFix;
    bool practiceFixEnabled = true;
    Recorder recorder;

    bool autoRec = false;
    bool showStatus = false;
    int statusLabelTag = 6000;
    auto createStatusLabel(CCLayer*);

    bool restartFlag = false;
    bool realTime = false;
    float speedhack = 1.f;

    ReplaySystem() : defFps(60), replay(defFps) {}

public:
    static auto& get() {
        static ReplaySystem instance;
        return instance;
    }

    ReplaySystem(const ReplaySystem&) = delete;

    inline auto& getReplay() { return replay; }
    inline auto getDefFps() { return defFps; }
    inline auto& getPracticeFix() { return practiceFix; }
    inline auto& getRecorder() { return recorder; }
    inline auto getSpeedhack() { return speedhack; }

    inline void setDefFps(int fps) { defFps = fps; }
    inline void setRestartFlag(bool a) { restartFlag = a; }
    inline void setSpeedhack(float speed) { speedhack = speed; }

    inline bool isPlaying() { return state == PLAYING; }
    inline bool isRecording() { return state == RECORDING; }
    inline bool isAutoRecording() { return autoRec; }
    inline bool isStatusLabel() { return showStatus; }
    inline bool isRestartFlag() { return restartFlag; }
    inline bool isRealTime() { return realTime; }
    inline bool isPracticeFix() { return practiceFixEnabled; }

    void togglePlaying();
    void toggleRecording();
    void toggleAutoRec() { autoRec = !autoRec; }
    void toggleStatusLabel() { showStatus = !showStatus; }
    void toggleRealTime() { realTime = !realTime; }
    void togglePracticeFix() { practiceFixEnabled = !practiceFixEnabled; }

    void updateFrameOffset();
    void updateStatusLabel();

    void resetState() {
        state = NOTHING;
        restartFlag = false;
        updateFrameOffset();
        updateStatusLabel();
    }

    void recordAction(bool, bool, bool flip = true);
    void playAction(const Action& action);
    bool handleRecord(bool, bool);
    void handlePlaying();

    void handleActivatedObj(bool, bool, GameObject*);

    void onReset();

    int getFrame();

    void Save();
    void Load();

    void autoSaveReplay(GJGameLevel*);
    void clearReplay();
};