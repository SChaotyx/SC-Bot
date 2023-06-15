#include "Hooks.h"

void CCScheduler_Update(CCScheduler* self, float dt) {
    matdash::orig<&CCScheduler_Update, matdash::Thiscall>(self, dt);
}

void GameObject_ActivateObject(GameObject* self, PlayerObject* player) {
    matdash::orig<&GameObject_ActivateObject>(self, player);
}

void PlayerObject_RingJump(PlayerObject* self, GameObject* ring) {
    matdash::orig<&PlayerObject_RingJump>(self, ring);
}

void GJBaseGameLayer_BumpPlayer(GJBaseGameLayer* self, PlayerObject* player, GameObject* object) {
    matdash::orig<&GJBaseGameLayer_BumpPlayer>(self, player, object);
}

bool LevelInfoLayer_Init(LevelInfoLayer* self, GJGameLevel* level) {
    matdash::orig<&LevelInfoLayer_Init>(self, level);
    return true;
}

bool EditLevelLayer_Init(EditLevelLayer* self, GJGameLevel* level) {
    matdash::orig<&EditLevelLayer_Init>(self, level);
    return true;
}

// PlayLayer Hooks

bool GDPlayLayer::Init(PlayLayer* self, GJGameLevel* level) {
    matdash::orig<&GDPlayLayer::Init>(self, level);
    return true;
}

void GDPlayLayer::Update(PlayLayer* self, float dt) {
    matdash::orig<&GDPlayLayer::Update, matdash::Thiscall>(self, dt);
}

void GDPlayLayer::UpdateVisibility(PlayLayer* self) {
    matdash::orig<&GDPlayLayer::UpdateVisibility>(self);
}

void GDPlayLayer::PauseGame(PlayLayer* self, bool unk) {
    matdash::orig<&GDPlayLayer::PauseGame>(self, unk);
}

void GDPlayLayer::LevelComplete(PlayLayer* self) {
    matdash::orig<&GDPlayLayer::LevelComplete>(self);
}

void GDPlayLayer::Quit(PlayLayer* self) {
    matdash::orig<&GDPlayLayer::Quit>(self);
}

void GDPlayLayer::Reset(PlayLayer*self) {
    matdash::orig<&GDPlayLayer::Reset>(self);
}

void GDPlayLayer::PushButton(PlayLayer*self, int unk, bool button) {
    matdash::orig<&GDPlayLayer::PushButton>(self, unk, button);
}

void GDPlayLayer::ReleaseButton(PlayLayer*self, int unk, bool button) {
    matdash::orig<&GDPlayLayer::ReleaseButton>(self, unk, button);
}

// CCObject* GDPlayLayer::CheckpointCreate() {}

// PauseLayer Hooks

bool GDPauseLayer::Init(PauseLayer* self) {
    matdash::orig<&GDPauseLayer::Init>(self);
    return true;
}

void GDPauseLayer::onResume(PauseLayer* self , CCObject* sender) {
    matdash::orig<&GDPauseLayer::onResume>(self, sender);
}

void GDPauseLayer::onRestart(PauseLayer* self, CCObject* sender) {
    matdash::orig<&GDPauseLayer::onRestart>(self, sender);
}

void GDPauseLayer::onEditor(PauseLayer* self, CCObject* sender) {
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

    matdash::add_hook<&GDPlayLayer::Init>(gd::base + 0x1FB780);
    matdash::add_hook<&GDPlayLayer::Update, matdash::Thiscall>(gd::base + 0x2029C0);
    matdash::add_hook<&GDPlayLayer::UpdateVisibility>(gd::base + 0x205460);
    matdash::add_hook<&GDPlayLayer::PauseGame>(gd::base + 0x20D3C0);
    matdash::add_hook<&GDPlayLayer::LevelComplete>(gd::base + 0x1FD3D0);
    matdash::add_hook<&GDPlayLayer::Quit>(gd::base + 0x20D810);
    matdash::add_hook<&GDPlayLayer::Reset>(gd::base + 0x20BF00);
    matdash::add_hook<&GDPlayLayer::PushButton>(gd::base + 0x111500);
    matdash::add_hook<&GDPlayLayer::ReleaseButton>(gd::base + 0x111660);
    //matdash::add_hook<&GDPlayLayer::CheckpointCreate, matdash::Optcall>(gd::base + 0x20ddd0);

    matdash::add_hook<&GDPauseLayer::Init>(gd::base + 0x1E4620);
    matdash::add_hook<&GDPauseLayer::onResume>(gd::base + 0x1e5fa0);
    matdash::add_hook<&GDPauseLayer::onRestart>(gd::base + 0x1e6040);
    matdash::add_hook<&GDPauseLayer::onEditor>(gd::base + 0x1E60E0); 
}