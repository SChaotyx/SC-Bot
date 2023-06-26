#include "ReplaySystem.h"
#include "Hooks.h"
#include "Utils.h"
#include "SCManager.h"

void ReplaySystem::recordAction(bool hold, bool player1, bool flip) {
    if(!(isRecording() || isAutoRecording())) return;
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
        GDPlayLayer::PushButtonO(GM->getPlayLayer(), 0, hold);
    else
        GDPlayLayer::ReleaseButtonO(GM->getPlayLayer(), 0, hold);
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
        if(GameManager::sharedState()->getPlayLayer())
            setRestartFlag(true);
    } else {
        setRestartFlag(false);
    }
    updateFrameOffset();
    updateStatusLabel();
}

void ReplaySystem::toggleRecording() {
    if(!isRecording()) {
        if(PlayLayer::get()) {
            if(!isPlaying()) {
                replay = Replay(defFps);
                setRestartFlag(true);
            } else setRestartFlag(false);
        } else replay = Replay(defFps);
    } else setRestartFlag(false);
    state = isRecording() ? NOTHING : RECORDING;
    updateFrameOffset();
    updateStatusLabel();
}

void ReplaySystem::onReset() {
    auto PL = GameManager::sharedState()->getPlayLayer();
    if(isPlaying()) {
        updateFrameOffset();
        GDPlayLayer::ReleaseButtonO(PL, 0, false);
        GDPlayLayer::ReleaseButtonO(PL, 0, true);
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
            if (isRecording() || isAutoRecording()) {
                for (const auto& object : practiceFix.activatedObjects) {
                    object->m_bHasBeenActivated = true;
                }
                for (const auto& object : practiceFix.activatedObjectsP2) {
                    object->m_bHasBeenActivatedP2 = true;
                }
            }
        }
        if(isRecording() || isAutoRecording()) {
            replay.removeActions(getFrame());
            const auto& actions = replay.getActions();
            bool holding = PL->m_pPlayer1->m_isHolding;
            if((holding && actions.empty()) || (!actions.empty() && actions.back().holdP1 != holding)) {
                recordAction(holding, true, false);
                if(holding) {
                    GDPlayLayer::ReleaseButtonO(PL, 0, true);
                    GDPlayLayer::PushButtonO(PL, 0, true);
                    PL->m_pPlayer1->m_hasJustHeld = true;
                }
            } else if (!actions.empty() && actions.back().holdP1 && holding && hasCheckpoints && checkpoint.player1.bufferOrb) {
                GDPlayLayer::ReleaseButtonO(PL, 0, true);
                GDPlayLayer::PushButtonO(PL, 0, true);
            }
            if(PlayLayer::get()->m_level->m_bTwoPlayerMode)
                recordAction(false, false, false);
            if(hasCheckpoints && !noPracticeFix) practiceFix.applyCheckpoint();
        }
    }
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

auto ReplaySystem::createStatusLabel(CCLayer* layer) {
    auto label = CCLabelBMFont::create("", "chatFont.fnt");
    label->setTag(statusLabelTag);
    label->setAnchorPoint({0, 0});
    label->setPosition(5, 5);
    label->setZOrder(999);
    layer->addChild(label);
    return label;
}

void ReplaySystem::updateStatusLabel() {
    if(auto PL = GameManager::sharedState()->getPlayLayer()) {
        auto label = cast<CCLabelBMFont*>(PL->getChildByTag(statusLabelTag));
        if(!label) label = createStatusLabel(PL);
        switch(state) {
            case NOTHING:
                label->setString("");
                break;
            case RECORDING:
                label->setString(showStatus ? "Recording" : "");
                break;
            case PLAYING:
                label->setString(showStatus ? "Playing" : "");
                break;
        }
    }
}

void ReplaySystem::autoSaveReplay(GJGameLevel* level){
    if(!isAutoRecording() || isPlaying()) return;

    auto t = std::time(nullptr);
    std::tm tm {};
    localtime_s(&tm, &t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d.%m.%Y-%H.%M.%S");
    auto date = oss.str();

    std::string levelID = std::to_string(level->m_nLevelID);
    std::string levelName = level->m_sLevelName;
    std::string creatorName = level->m_sCreatorName;
    std::string folder = "SCReplays";

    std::string path =
        folder+"\\["+levelID+"]["+date+"]["+levelName+"]["+creatorName+"].screp";
    
    if(!std::filesystem::is_directory(folder) || !std::filesystem::exists(folder))
        std::filesystem::create_directory(folder);

    getReplay().Save(path);
}

void ReplaySystem::clearReplay() {
    if(isAutoRecording() && (!isRecording() && !isPlaying())) {
        replay = Replay(defFps);
    }
}

void ReplaySystem::modifySpeedhack(bool inc) {
    float actual_speed = getSpeedhack();
    if(inc) {
        if(actual_speed < 4.f) setSpeedhack(actual_speed + 0.100000000000f);
    } else {
        if(actual_speed > 0.1f) setSpeedhack(actual_speed - 0.100000000000f);
    }
    if(getSpeedhack() < 0.1f) setSpeedhack(0.1f);
    if(getSpeedhack() > 4.f) setSpeedhack(4.f);
    if(PlayLayer::get()) updateStatusLabel();
}

void ReplaySystem::modifyDefFps(bool inc) {
    int deffps = getDefFps();
    if(inc) {
        if(deffps < 360) setDefFps(deffps + 60);
    } else {
        if(deffps > 60) setDefFps(deffps - 60);
    }
    if(getDefFps() < 60) setDefFps(60);
    if(getDefFps() > 360) setDefFps(360);
    if(PlayLayer::get()) updateStatusLabel();
}

void ReplaySystem::Load() {
    if(SCManager::getSCModString("Bot_fps") != "")
        defFps = std::stoi(SCManager::getSCModString("Bot_fps"));
    if(SCManager::getSCModString("Bot_speedhack") != "")
        speedhack = std::stof(SCManager::getSCModString("Bot_speedhack"));
    autoRec = SCManager::getSCModVariable("Bot_autoRec");
    showStatus = SCManager::getSCModVariable("Bot_showStatus");
    realTime = SCManager::getSCModVariable("Bot_realTime");
    noPracticeFix = SCManager::getSCModVariable("Bot_noPracticeFix");

    if(SCManager::getSCModString("Rec_width") != "")
        recorder.m_width = std::stoi(SCManager::getSCModString("Rec_width"));
    if(SCManager::getSCModString("Rec_height") != "")
        recorder.m_height = std::stoi(SCManager::getSCModString("Rec_height"));
    if(SCManager::getSCModString("Rec_fps") != "")
        recorder.m_fps = std::stoi(SCManager::getSCModString("Rec_fps"));
    if(SCManager::getSCModString("Rec_untilEndDuration") != "")
        recorder.m_afterEndDuration = std::stof(SCManager::getSCModString("Rec_untilEndDuration"));
    if(SCManager::getSCModString("Rec_bitrate") != "")
        recorder.m_bitrate = SCManager::getSCModString("Rec_bitrate");
    if(SCManager::getSCModString("Rec_codec") != "")
        recorder.m_codec = SCManager::getSCModString("Rec_codec");
    if(SCManager::getSCModString("Rec_extraArgs") != "")
        recorder.m_extraArgs = SCManager::getSCModString("Rec_extraArgs");
    if(SCManager::getSCModString("Rec_extraAudioArgs") != "")
        recorder.m_extraAudioArgs = SCManager::getSCModString("Rec_extraAudioArgs");
    recorder.m_untilEnd = SCManager::getSCModVariable("Rec_untilEnd");
    recorder.m_includeAudio = SCManager::getSCModVariable("Rec_includeAudio");
}

void ReplaySystem::Save() {
    SCManager::setSCModString("Bot_fps", std::to_string(defFps));
    SCManager::setSCModString("Bot_speedhack", std::to_string(speedhack));
    SCManager::setSCModVariable("Bot_autoRec", autoRec);
    SCManager::setSCModVariable("Bot_showStatus", showStatus);
    SCManager::setSCModVariable("Bot_realTime", realTime);
    SCManager::setSCModVariable("Bot_noPracticeFix", noPracticeFix);

    SCManager::setSCModString("Rec_width", std::to_string(recorder.m_width));
    SCManager::setSCModString("Rec_height", std::to_string(recorder.m_height));
    SCManager::setSCModString("Rec_fps", std::to_string(recorder.m_fps));
    SCManager::setSCModString("Rec_bitrate", recorder.m_bitrate);
    SCManager::setSCModString("Rec_untilEndDuration", std::to_string(recorder.m_afterEndDuration));
    SCManager::setSCModString("Rec_codec", recorder.m_codec);
    SCManager::setSCModString("Rec_extraArgs", recorder.m_extraArgs);
    SCManager::setSCModString("Rec_extraAudioArgs", recorder.m_extraAudioArgs);
    SCManager::setSCModVariable("Rec_untilEnd", recorder.m_untilEnd);
    SCManager::setSCModVariable("Rec_includeAudio", recorder.m_includeAudio);
}