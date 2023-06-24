#include "Hooks.h"
#include "ReplaySystem.h"
#include "ReplayLayer.h"
#include <chrono>
#include <thread>

float dif_dt = 0.f;

void CCScheduler_Update(CCScheduler* self, float dt) {
    auto& RS = ReplaySystem::get();
    auto PL = PlayLayer::get();
    if(PL && !PL->m_bIsPaused && (RS.isRecording() || RS.isPlaying() || RS.isAutoRecording() || RS.getRecorder().m_recording)) {
        const auto fps = RS.getReplay().getFps();
        auto speedhack = RS.getSpeedhack();
        float target_dt = 1.f / fps;
        float actual_dt = static_cast<float>(CCDirector::sharedDirector()->getAnimationInterval());

        if(speedhack > 1.f) {
            dt *= speedhack;
            actual_dt *= speedhack;
            target_dt *= speedhack;
        }

        // pinche megahack
        if(actual_dt < target_dt) {
            matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, dt);
            int static alerted = 0;
            if(alerted > 2) {
                matdash::orig<&GDPlayLayer::PauseGame>(PL, true);
                FLAlertLayer::create(nullptr, "Warning!", "Ok", nullptr, "Please disable your FPS/TPS bypass or match with actual replay FPS.")->show();
                alerted = 0;
                return; 
            }
            alerted ++;
            return; 
        }
        
        if(dt == 0.f) return matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, target_dt);
        unsigned times = static_cast<int>(actual_dt / target_dt);
        if(RS.isRealTime() || RS.getRecorder().m_recording || RS.getSpeedhack() < 1.f) {
            for(unsigned i = 0; i < times; ++i) {
                if(speedhack > 0 && speedhack < 1) {
                    // bad idea? work fine lol
                    float delayInSeconds = (target_dt / speedhack) - target_dt;
                    std::chrono::milliseconds delay(static_cast<long long>(delayInSeconds * 1000));
                    std::this_thread::sleep_for(delay);
                    matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, target_dt);
                } else 
                    matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, target_dt);
            }
            return;
        } else {
            dif_dt += dt - actual_dt;
            if(RS.isPlaying()){
                // lag spike dont break replaying but become unstable if it's so laggy
                while(dif_dt > target_dt) { ++times; dif_dt = dif_dt - target_dt; }
                for(unsigned i = 0; i < times; ++i) {
                    matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, target_dt);
                }
            } else {
                // more stable but break replaying
                for(unsigned i = 0; i < times; ++i)
                    matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, target_dt + (dif_dt / times));
                dif_dt = 0.f;
            }
            return;
        }
    } else {
        return matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, dt);
    }
}

void GameObject_ActivateObject(GameObject* self, PlayerObject* player) {
    auto& RS = ReplaySystem::get();
    bool a = self->m_bHasBeenActivated;
    bool b = self->m_bHasBeenActivatedP2;
    matdash::orig<&GameObject_ActivateObject>(self, player);
    RS.handleActivatedObj(a, b, self);
}

void PlayerObject_RingJump(PlayerObject* self, GameObject* ring) {
    auto& RS = ReplaySystem::get();
    bool a = ring->m_bHasBeenActivated;
    bool b = ring->m_bHasBeenActivatedP2;
    matdash::orig<&PlayerObject_RingJump>(self, ring);
    RS.handleActivatedObj(a, b, ring);
}

void GJBaseGameLayer_BumpPlayer(GJBaseGameLayer* self, PlayerObject* player, GameObject* object) {
    auto& RS = ReplaySystem::get();
    bool a = object->m_bHasBeenActivated;
    bool b = object->m_bHasBeenActivatedP2;
    matdash::orig<&GJBaseGameLayer_BumpPlayer>(self, player, object);
    RS.handleActivatedObj(a, b, object);
}

bool LevelInfoLayer_Init(LevelInfoLayer* self, GJGameLevel* level) {
    matdash::orig<&LevelInfoLayer_Init>(self, level);

    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->getWinSize();

    auto menu = CCMenu::create();
    menu->setPosition(0,0);
    self->addChild(menu);

    auto sprite = CCSprite::create("SC_ReplayBtn_001.png");
    auto button = CCMenuItemSpriteExtra::create(sprite, self, menu_selector(ReplayLayer::OpenCallback));
    button->setScale(0.85f);
    button->m_fBaseScale = 0.85f;
    button->setPosition(120, winSize.height - 23);
    menu->addChild(button);

    return true;
}

bool EditLevelLayer_Init(EditLevelLayer* self, GJGameLevel* level) {
    matdash::orig<&EditLevelLayer_Init>(self, level);

    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->getWinSize();

    auto menu = CCMenu::create();
    menu->setPosition(0,0);
    self->addChild(menu);

    auto sprite = CCSprite::create("SC_ReplayBtn_001.png");
    auto button = CCMenuItemSpriteExtra::create(sprite, self, menu_selector(ReplayLayer::OpenCallback));
    button->setScale(0.85f);
    button->m_fBaseScale = 0.85f;
    button->setPosition(70, winSize.height - 23);
    menu->addChild(button);

    return true;
}

bool MenuLayer_Init(MenuLayer* self) {
    matdash::orig<&MenuLayer_Init>(self);
    static bool hasLoaded = false;
    if(!hasLoaded) {
        ReplaySystem::get().Load();
        hasLoaded = true;
    }
    return true;
}

// PlayLayer Hooks

bool GDPlayLayer::Init(PlayLayer* self, GJGameLevel* level) {
    matdash::orig<&GDPlayLayer::Init>(self, level);
    auto& RS = ReplaySystem::get();
    RS.clearReplay();
    RS.updateStatusLabel();
    if(RS.getRecorder().m_recording) 
        RS.getRecorder().Start(RS.getRecorder().videoPath);
    RS.getRecorder().updateSongOffset(self);
    dif_dt = 0.f;
    return true;
}

void GDPlayLayer::Update(PlayLayer* self, float dt) {
    auto& RS = ReplaySystem::get();
    if(RS.isPlaying()) RS.handlePlaying();
    if(RS.getRecorder().m_recording)
        RS.getRecorder().handleRecording(self, dt);
    matdash::orig<&GDPlayLayer::Update, matdash::Thiscall>(self, dt);
}

void GDPlayLayer::UpdateVisibility(PlayLayer* self) {
    matdash::orig<&GDPlayLayer::UpdateVisibility>(self);
}

void GDPlayLayer::PauseGame(PlayLayer* self, bool unk) {
    auto& RS = ReplaySystem::get();
    if(RS.isRecording()) RS.recordAction(false, true, false);
    matdash::orig<&GDPlayLayer::PauseGame>(self, unk);
    dif_dt = 0.f;
}

void GDPlayLayer::LevelComplete(PlayLayer* self) {
    auto& RS = ReplaySystem::get();
    if(self->m_level->m_eLevelType != kGJLevelTypeEditor) {
        if(!self->m_isPracticeMode) RS.autoSaveReplay(self->m_level);
    }
    RS.resetState();
    matdash::orig<&GDPlayLayer::LevelComplete>(self);
}

void GDPlayLayer::Quit(PlayLayer* self) {
    ReplaySystem::get().resetState();
    matdash::orig<&GDPlayLayer::Quit>(self);
    dif_dt = 0.f;
}

void GDPlayLayer::Reset(PlayLayer*self) {
    auto& RS = ReplaySystem::get();
    matdash::orig<&GDPlayLayer::Reset>(self);
    RS.onReset();
    RS.getRecorder().updateSongOffset(self);
    dif_dt = 0.f;
}

void GDPlayLayer::PushButton(PlayLayer*self, int unk, bool button) {
    auto& RS = ReplaySystem::get();
    if(RS.handleRecord(true, button)) return;
    matdash::orig<&GDPlayLayer::PushButton>(self, unk, button);
}

void GDPlayLayer::ReleaseButton(PlayLayer*self, int unk, bool button) {
    auto& RS = ReplaySystem::get();
    if(RS.handleRecord(false, button)) return;
    matdash::orig<&GDPlayLayer::ReleaseButton>(self, unk, button);
}

void GDPlayLayer::PushButtonO(PlayLayer*self, int unk, bool button)
{ matdash::orig<&GDPlayLayer::PushButton>(self, unk, button); }

void GDPlayLayer::ReleaseButtonO(PlayLayer*self, int unk, bool button)
{ matdash::orig<&GDPlayLayer::ReleaseButton>(self, unk, button); }

CCObject* GDPlayLayer::CheckpointCreate() {
    return CheckpointObjectMod::create();
}

// PauseLayer Hooks

bool GDPauseLayer::Init(PauseLayer* self) {
    matdash::orig<&GDPauseLayer::Init>(self);

    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->getWinSize();

    auto menu = CCMenu::create();
    menu->setPosition(0,0);
    self->addChild(menu);

    auto sprite = CCSprite::create("SC_ReplayBtn_001.png");
    auto button = CCMenuItemSpriteExtra::create(sprite, self, menu_selector(ReplayLayer::OpenCallback));
    button->setScale(0.85f);
    button->m_fBaseScale = 0.85f;
    button->setPosition(50, winSize.height - 130);
    menu->addChild(button);

    return true;
}

void GDPauseLayer::onResume(PauseLayer* self , CCObject* sender) {
    matdash::orig<&GDPauseLayer::onResume>(self, sender);
    auto& RS = ReplaySystem::get();
    if(RS.isRestartFlag()) {
        auto PL = GameManager::sharedState()->getPlayLayer();
        matdash::orig<&GDPlayLayer::Reset>(PL);
        RS.setRestartFlag(false);
    }
    dif_dt = 0.f;
}

void GDPauseLayer::onRestart(PauseLayer* self, CCObject* sender) {
    matdash::orig<&GDPauseLayer::onRestart>(self, sender);
    ReplaySystem::get().setRestartFlag(false);
}

void GDPauseLayer::onEditor(PauseLayer* self, CCObject* sender) {
    ReplaySystem::get().resetState();
    matdash::orig<&GDPauseLayer::onEditor>(self, sender);
}

auto cocos(const char* symbol) {
    static auto mod = GetModuleHandleA("libcocos2d.dll");
    return GetProcAddress(mod, symbol);
}

void Hooks::Init() {
    matdash::add_hook<&CCScheduler_Update, matdash::Thiscall>(cocos("?update@CCScheduler@cocos2d@@UAEXM@Z"));
    matdash::add_hook<&GameObject_ActivateObject>(gd::base + 0xef0e0);
    matdash::add_hook<&PlayerObject_RingJump>(gd::base + 0x1f4ff0);
    matdash::add_hook<&GJBaseGameLayer_BumpPlayer>(gd::base + 0x10ed50);

    matdash::add_hook<&LevelInfoLayer_Init>(gd::base + 0x175df0);
    matdash::add_hook<&EditLevelLayer_Init>(gd::base + 0x6f5d0);
    matdash::add_hook<&MenuLayer_Init>(gd::base + 0x1907B0);

    matdash::add_hook<&GDPlayLayer::Init>(gd::base + 0x1FB780);
    matdash::add_hook<&GDPlayLayer::Update, matdash::Thiscall>(gd::base + 0x2029C0);
    matdash::add_hook<&GDPlayLayer::UpdateVisibility>(gd::base + 0x205460);
    matdash::add_hook<&GDPlayLayer::PauseGame>(gd::base + 0x20D3C0);
    matdash::add_hook<&GDPlayLayer::LevelComplete>(gd::base + 0x1FD3D0);
    matdash::add_hook<&GDPlayLayer::Quit>(gd::base + 0x20D810);
    matdash::add_hook<&GDPlayLayer::Reset>(gd::base + 0x20BF00);
    matdash::add_hook<&GDPlayLayer::PushButton>(gd::base + 0x111500);
    matdash::add_hook<&GDPlayLayer::ReleaseButton>(gd::base + 0x111660);
    matdash::add_hook<&GDPlayLayer::CheckpointCreate, matdash::Optcall>(gd::base + 0x20ddd0);

    matdash::add_hook<&GDPauseLayer::Init>(gd::base + 0x1E4620);
    matdash::add_hook<&GDPauseLayer::onResume>(gd::base + 0x1e5fa0);
    matdash::add_hook<&GDPauseLayer::onRestart>(gd::base + 0x1e6040);
    matdash::add_hook<&GDPauseLayer::onEditor>(gd::base + 0x1E60E0); 
}