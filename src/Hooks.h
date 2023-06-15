#pragma once

#include "includes.h"

namespace Hooks {
    void Init();
}

void CCScheduler_Update(CCScheduler*, float);
void GameObject_ActivateObject(GameObject*, PlayerObject*);
void PlayerObject_RingJump(PlayerObject*, GameObject*);
void GJBaseGameLayer_BumpPlayer(GJBaseGameLayer*, PlayerObject*, GameObject*);

bool LevelInfoLayer_Init(LevelInfoLayer*, GJGameLevel*);
bool EditLevelLayer_Init(EditLevelLayer*, GJGameLevel*);

namespace GDPlayLayer {
    bool Init(PlayLayer*, GJGameLevel*);
    void Update(PlayLayer*, float);
    void UpdateVisibility(PlayLayer*);
    void PauseGame(PlayLayer*, bool);
    void LevelComplete(PlayLayer*);
    void Quit(PlayLayer*);
    void Reset(PlayLayer*);
    void PushButton(PlayLayer*, int, bool);
    void ReleaseButton(PlayLayer*, int, bool);
    CCObject* CheckpointCreate();
}

namespace GDPauseLayer {
    bool Init(PauseLayer*);
    void onResume(PauseLayer*, CCObject*);
    void onRestart(PauseLayer*, CCObject*);
    void onEditor(PauseLayer*, CCObject*);
}