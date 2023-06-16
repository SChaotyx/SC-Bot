#include "PracticeFix.h"
#include "ReplaySystem.h"

void CheckpointObjectMod::saveInfo() {
    auto& RS = ReplaySystem::get();
    m_checkpoint = RS.getPracticeFix().createCheckpoint();
}

Checkpoint PracticeFix::createCheckpoint() {
    auto PL = GameManager::sharedState()->getPlayLayer();
    if(PL) {
        auto& RS = ReplaySystem::get();
        return {
            CheckpointData::fromPlayer(PL->m_pPlayer1),
            CheckpointData::fromPlayer(PL->m_pPlayer2),
            activatedObjects.size(),
            activatedObjectsP2.size(),
            PL->m_time
        };
    } else {
        return {};
    }
}