#pragma once

#include "includes.h"
#include "Nodes.h"

class ReplayLayer : public FLAlertLayer, public FLAlertLayerProtocol {
    NumberInputNode* m_fpsInput;
    CCLabelBMFont* m_replayInfo;
    
    void updateReplayInfo();
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
    void keyDown(enumKeyCodes key) { if(key == 27) keyBackClicked(); }

    void onRecord(CCObject*);
    void onPlay(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);
    void onRender(CCObject*);

    void onToggleAutoRec(CCObject*);
    void onToggleStatusLabel(CCObject*);
    void onToggleRealTime(CCObject*);
    void onOpenRepFolder(CCObject*);

    void loadReplay();
};