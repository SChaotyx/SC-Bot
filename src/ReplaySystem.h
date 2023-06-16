#pragma once

#include "includes.h"
#include "Replay.h"
#include "PracticeFix.h"

enum RSState {
    NOTHING,
    RECORDING,
    PLAYING
};

enum PlayMode {
    xpos,
    frame
};

class ReplaySystem {
    Replay replay;
    RSState state = NOTHING;
    int defFps;
    ReplayType defType;
    PlayMode playMode = PlayMode::frame;
    bool autoRec = false;
    int frameOffset = 0;
    size_t actionIndex = 0;

    PracticeFix practiceFix;

    ReplaySystem() : defFps(60), replay(defFps), defType(replay.getType()) {}

public:
    static auto& get() {
        static ReplaySystem instance;
        return instance;
    }

    ReplaySystem(const ReplaySystem&) = delete;

    inline auto& getReplay() { return replay; }
    inline auto getDefFps() { return defFps; }
    inline auto getDefType() { return defType; }
    inline auto& getPracticeFix() { return practiceFix; }

    inline void setDefFps(int fps) { defFps = fps; }
    inline void setDefType(ReplayType type) { defType = type; }

    inline bool isPlaying() { return state == PLAYING; }
    inline bool isRecording() { return state == RECORDING; }
    inline bool isAutoRecEnabled() { return autoRec == true; }

    void togglePlaying();
    void toggleRecording();

    void updateFrameOffset();
    void updateStatusLabel();

    void resetState() {
        state = NOTHING;
        updateFrameOffset();
    }

    void recordAction(bool, bool, bool flip = true);
    void playAction(const Action& action);
    bool handleRecord(bool, bool);
    void handlePlaying();

    void handleActivatedObj(bool, bool, GameObject*);

    void onReset();

    int getCurrentFrame();
    int getActionFrame(double);

    void Save();
    void Load();
};