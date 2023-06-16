#pragma once

#include "includes.h"

struct Action {
    int frame;
    bool holdP1;
    bool holdP2;
};


class Replay {
protected:
    std::vector<Action> actions;
    int fps;

public:
    Replay(int fps) : fps(fps) {}

    int getFps() { return fps; }
    auto& getActions() { return actions; }

    void addAction(const Action& action) { actions.push_back(action); }
    void removeActions(int);

    void Save(const std::string& path);
    static Replay Load(const std::string& path);
};