#pragma once

#include "includes.h"
#include "Nodes.h"

class RecorderLayer : public FLAlertLayer, public FLAlertLayerProtocol {
    NumberInputNode* m_widthInput;
    NumberInputNode* m_heightInput;
public:
    RecorderLayer* create() {
        auto pRet = new RecorderLayer();
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

    void onRender(CCObject*);
    void onToggleUntilEnd(CCObject*);
    void onToggleIncludeAudio(CCObject*);
    void getWinSize(CCObject*);
};