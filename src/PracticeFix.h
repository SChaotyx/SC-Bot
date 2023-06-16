#pragma once

#include <stack>
#include "includes.h"

struct CheckpointData {
    double yAccel;
    float rotation;
    bool bufferOrb;
    static CheckpointData fromPlayer(PlayerObject* player) {
        return { player->m_yAccel, player->getRotation(), player->m_hasJustHeld };
    };
    void apply(PlayerObject* player) const {
        player->m_yAccel = yAccel;
        player->setRotation(rotation);
    }
};

struct Checkpoint {
    CheckpointData player1;
    CheckpointData player2;
    size_t activatedObjectsSize;
    size_t activatedObjectsP2Size;
    int frame;
};

class CheckpointObjectMod : public CheckpointObject {
public:
    Checkpoint m_checkpoint;
    void saveInfo();
    static auto create() {
        auto ret = new CheckpointObjectMod;
        if (ret && ret->init()) {
            ret->autorelease();
            ret->saveInfo();
        } else {
            CC_SAFE_DELETE(ret);
        }
        return ret;
    }
};

class PracticeFix {
    std::stack<Checkpoint> checkpoint;
    std::vector<GameObject*> activatedObjects;
    std::vector<GameObject*> activatedObjectsP2;
    friend class ReplaySystem;
public:
    Checkpoint createCheckpoint();
    Checkpoint getLastCheckpoint() {
        auto PL = GameManager::sharedState()->getPlayLayer();
        if(PL && PL->m_checkpoints->count()) {
            auto checkpointObj = dynamic_cast<CheckpointObjectMod*>(PL->m_checkpoints->lastObject());
            if(checkpointObj) return checkpointObj->m_checkpoint;
        }
        return {};
    }

    void applyCheckpoint() {
        auto PL = GameManager::sharedState()->getPlayLayer();
        if(PL) {
            const auto checkpoint = getLastCheckpoint();
            checkpoint.player1.apply(PL->m_pPlayer1);
            checkpoint.player2.apply(PL->m_pPlayer2);
        }
    }

    void addActivatedObject(GameObject* object) {
        activatedObjectsP2.push_back(object);
    }

    void addActivatedObjectP2(GameObject* object) {
        activatedObjectsP2.push_back(object);
    }
};
