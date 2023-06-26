#pragma once

#include "includes.h"
#include "Nodes.h"

class ReplayLayer : public FLAlertLayer, public FLAlertLayerProtocol {
    CCLabelBMFont* m_fpsInput;
    CCLabelBMFont* m_spdhkInput;
    CCLabelBMFont* m_replayInfo;
    
    void updateReplayInfo();
    void updateFpsInput();
    void updateSpdhkInput();
    virtual void FLAlert_Clicked(FLAlertLayer*, bool);
    void showOverwriteAlert(int);

    public:
    ReplayLayer* create() {
        auto pRet = new ReplayLayer();
        if(pRet && pRet->Init()) {
            pRet->autorelease();
            return pRet;
        }
        CC_SAFE_DELETE(pRet);
        return nullptr;
    }
    bool Init();
    void OpenCallback(CCObject*);
    void CloseCallback(CCObject*) { keyBackClicked(); }
    void keyBackClicked();
    void keyDown(enumKeyCodes key);

    void onRecord(CCObject*);
    void onPlay(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);

    void onModifyFps(CCObject*);
    void onModifySpdhk(CCObject*);

    void onToggleAutoRec(CCObject*);
    void onToggleStatusLabel(CCObject*);
    void onToggleRealTime(CCObject*);
    void onOpenRepFolder(CCObject*);
    void onTogglePracticeFix(CCObject*);

    void loadReplay();
};