#pragma once

#include "includes.h"

class ReplayLayer : public FLAlertLayer, public FLAlertLayerProtocol {
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
    void keyBackClicked() { FLAlertLayer::keyBackClicked(); }
    void keyDown(enumKeyCodes key) { if(key == 27) keyBackClicked(); }

    void onRecord(CCObject*);
    void onPlay(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);
    void onRender(CCObject*);
};