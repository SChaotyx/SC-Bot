#include "ReplaySystem.h"
#include "Hooks.h"

void ReplaySystem::recordAction(bool hold, bool player1, bool flip) {
    if(!(isRecording() || isAutoRecEnabled())) return;
    auto GM = GameManager::sharedState();
    auto PL = GM->getPlayLayer();
    auto isTwoPlayer = PlayLayer::get()->m_level->m_bTwoPlayerMode;
    player1 ^= flip && GM->getGameVariable("0010");
    Action action;
    action.holdP1 = hold;
    action.holdP2 = isTwoPlayer && !player1;
    action.frame = getFrame();
    std::cout << action.frame <<  " | " << action.holdP1 << " | " << action.holdP2 << std::endl;
    replay.addAction(action);
}

bool ReplaySystem::handleRecord(bool hold, bool button) {
    if(GameManager::sharedState()->getPlayLayer()) {
        if(isPlaying()) return true;
        recordAction(hold, button, true);
    }
    return false;
}

void ReplaySystem::playAction(const Action& action) {
    auto GM = GameManager::sharedState();
    auto flip = GM->getGameVariable("0010");
    bool hold = !action.holdP2 ^ flip; // why crash wtf
    if (action.holdP1)
        matdash::orig<&GDPlayLayer::PushButton>(GM->getPlayLayer(), 0, hold);
    else
        matdash::orig<&GDPlayLayer::ReleaseButton>(GM->getPlayLayer(), 0, hold);
}

void ReplaySystem::handlePlaying() {
    if(!isPlaying()) return;
    auto& actions = replay.getActions();
    Action action;
    while (actionIndex < actions.size() && getFrame() >= (action = actions[actionIndex]).frame) {
        playAction(action);
        ++actionIndex;
    }
}

int ReplaySystem::getFrame() {
    auto PL = GameManager::sharedState()->getPlayLayer();
    if(PL != nullptr)
        return static_cast<int>(PL->m_time * replay.getFps()) + frameOffset;
        return 0;
}

void ReplaySystem::togglePlaying() {
    state = isPlaying() ? NOTHING : PLAYING;
    auto PL = GameManager::sharedState()->getPlayLayer();
    if(isPlaying()) {
        actionIndex = 0;
        std::cout << "isPlaying" << std::endl;
    }
    updateFrameOffset();
}

void ReplaySystem::toggleRecording() {
    state = isRecording() ? NOTHING : RECORDING;
    if(isRecording()) {
        replay = Replay(defFps);
        std::cout << "isRecording" << std::endl;
    }
    updateFrameOffset();
}

void ReplaySystem::onReset() {
    std::cout << replay.getActions().size() << std::endl;
    auto PL = GameManager::sharedState()->getPlayLayer();
    if(isPlaying()) {
        updateFrameOffset();
        matdash::orig<&GDPlayLayer::ReleaseButton>(PL, 0, false);
        matdash::orig<&GDPlayLayer::ReleaseButton>(PL, 0, true);
        actionIndex = 0;
        practiceFix.activatedObjects.clear();
        practiceFix.activatedObjectsP2.clear();
    } else {
        bool hasCheckpoints = PL->m_checkpoints->count();
        const auto checkpoint = practiceFix.getLastCheckpoint();
        if(!hasCheckpoints) {
            practiceFix.activatedObjects.clear();
            practiceFix.activatedObjectsP2.clear();
            frameOffset = 0;
        } else {
            frameOffset = checkpoint.frame;
            constexpr auto deleteFrom = [&](auto& vec, size_t index) {
                vec.erase(vec.begin() + index, vec.end());
            };
            deleteFrom(practiceFix.activatedObjects, checkpoint.activatedObjectsSize);
            deleteFrom(practiceFix.activatedObjectsP2, checkpoint.activatedObjectsP2Size);
            if (isRecording() || isAutoRecEnabled()) {
                for (const auto& object : practiceFix.activatedObjects) {
                    object->m_bHasBeenActivated = true;
                }
                for (const auto& object : practiceFix.activatedObjectsP2) {
                    object->m_bHasBeenActivatedP2 = true;
                }
            }
        }
        if(isRecording() || isAutoRecEnabled()) {
            replay.removeActions(getFrame());
            const auto& actions = replay.getActions();
            bool holding = PL->m_pPlayer1->m_isHolding;
            if((holding && actions.empty()) || (!actions.empty() && actions.back().holdP1 != holding)) {
                recordAction(holding, true, false);
                if(holding) {
                    matdash::orig<&GDPlayLayer::ReleaseButton>(PL, 0, true);
                    matdash::orig<&GDPlayLayer::PushButton>(PL, 0, true);
                    PL->m_pPlayer1->m_hasJustHeld = true;
                }
            } else if (!actions.empty() && actions.back().holdP1 && holding && hasCheckpoints && checkpoint.player1.bufferOrb) {
                matdash::orig<&GDPlayLayer::ReleaseButton>(PL, 0, true);
                matdash::orig<&GDPlayLayer::PushButton>(PL, 0, true);
            }
            if(PlayLayer::get()->m_level->m_bTwoPlayerMode)
                recordAction(false, false, false);
            if(hasCheckpoints) practiceFix.applyCheckpoint();
        }
    }
    std::cout << replay.getActions().size() << std::endl;
}

void ReplaySystem::updateFrameOffset() {
    frameOffset = practiceFix.getLastCheckpoint().frame;
}

void ReplaySystem::handleActivatedObj(bool a , bool b, GameObject* object) {
    auto PL = GameManager::sharedState()->getPlayLayer();
    if(PL && PL->m_isPracticeMode && isRecording()) {
        if(object->m_bHasBeenActivated && !a)
            practiceFix.addActivatedObject(object);
        if(object->m_bHasBeenActivatedP2 && !b)
            practiceFix.addActivatedObjectP2(object);
    }
}

