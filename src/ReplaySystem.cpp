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
    action.xpos = PL->m_pPlayer1->m_position.x;
    action.time = PL->m_time;
    std::cout << action.time << " | " << PL->m_time << std::endl;
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
    //std::cout << GameManager::sharedState()->getPlayLayer()->m_time;
    if(!isPlaying()) return;
    auto xpos = GameManager::sharedState()->getPlayLayer()->m_pPlayer1->m_position.x;
    auto& actions = replay.getActions();
    Action action;
    // here get play type instead of replay type because time and xpos is stored
    if(playMode == PlayMode::xpos) {
         while (actionIndex < actions.size() && xpos <= (action = actions[actionIndex]).xpos) {
            playAction(action);
            ++actionIndex;
         }
    } else {
        while (actionIndex < actions.size() && getCurrentFrame() >= getActionFrame((action = actions[actionIndex]).time)) {
            playAction(action);
            ++actionIndex;
        }
    }
}

int ReplaySystem::getCurrentFrame() {
    auto PL = GameManager::sharedState()->getPlayLayer();
    if(PL != nullptr) {
        return static_cast<int>(PL->m_time * getDefFps()) + frameOffset;
    }
    return 0;
}

int ReplaySystem::getActionFrame(double time) {
    //get fps from rp system not from replay
    return static_cast<int>(time * getDefFps());
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
        replay = Replay(defFps, defType);
        std::cout << "isRecoring" << std::endl;
    }
    updateFrameOffset();
}

void ReplaySystem::onReset() {
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
            frameOffset = getActionFrame(checkpoint.time);
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
            if(replay.getType() == ReplayType::XPOS)
                replay.removeActions(PL->m_pPlayer1->m_position.x);
            else
                replay.removeActions(PL->m_time);
            const auto& actions = replay.getActions();
            bool holding = PL->m_pPlayer1->m_isHolding;
            if((holding && actions.empty()) || (!actions.empty() && actions.back().holdP1 != holding)) {
                matdash::orig<&GDPlayLayer::ReleaseButton>(PL, 0, true);
                matdash::orig<&GDPlayLayer::PushButton>(PL, 0, true);
            }
            if(PlayLayer::get()->m_level->m_bTwoPlayerMode)
                recordAction(false, false, false);
            if(hasCheckpoints) practiceFix.applyCheckpoint();
        }
    }
}

void ReplaySystem::updateFrameOffset() {
    frameOffset = getActionFrame(practiceFix.getLastCheckpoint().time);
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

