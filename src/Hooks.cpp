#include "Hooks.h"
#include "ReplaySystem.h"
#include "ReplayLayer.h"

void CCScheduler_Update(CCScheduler* self, float dt) {
    matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, dt);
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

bool MenuLayer_Init() {
    matdash::orig<&MenuLayer_Init>();
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
    return true;
}

void GDPlayLayer::Update(PlayLayer* self, float dt) {
    auto& RS = ReplaySystem::get();
    if(RS.isPlaying()) RS.handlePlaying();
    matdash::orig<&GDPlayLayer::Update, matdash::Thiscall>(self, dt);
}

void GDPlayLayer::UpdateVisibility(PlayLayer* self) {
    matdash::orig<&GDPlayLayer::UpdateVisibility>(self);
}

void GDPlayLayer::PauseGame(PlayLayer* self, bool unk) {
    auto& RS = ReplaySystem::get();
    if(RS.isRecording()) RS.recordAction(false, true, false);
    matdash::orig<&GDPlayLayer::PauseGame>(self, unk);
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
}

void GDPlayLayer::Reset(PlayLayer*self) {
    auto& RS = ReplaySystem::get();
    matdash::orig<&GDPlayLayer::Reset>(self);
    RS.onReset();
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
    //check if required restart on resume
    matdash::orig<&GDPauseLayer::onResume>(self, sender);
}

void GDPauseLayer::onRestart(PauseLayer* self, CCObject* sender) {
    //turn false restart flag
    matdash::orig<&GDPauseLayer::onRestart>(self, sender);
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