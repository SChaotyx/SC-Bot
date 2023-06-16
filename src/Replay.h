#pragma once

#include "includes.h"

enum ReplayType {
    XPOS,
    FRAME // time based
};

struct Action {
    double time; // for multi fps support 
    float xpos;
    bool holdP1;
    bool holdP2;
};


class Replay {
protected:
    std::vector<Action> actions;
    int fps;
    ReplayType type;

public:
    Replay(int fps, ReplayType type = ReplayType::FRAME) : fps(fps), type(type) {}

    int getFps() { return fps; }
    auto getType() { return type; }
    auto& getActions() { return actions; }

    void addAction(const Action& action) { actions.push_back(action); }
    void removeActions(double time);

    void Save(const std::string& path);
    static Replay Load(const std::string& path);
};