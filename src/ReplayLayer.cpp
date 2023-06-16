#include "ReplayLayer.h"
#include "ReplaySystem.h"

bool ReplayLayer::Init() {
    if(!initWithColor(ccc4(0, 0, 0, 130))) return false;

    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->getWinSize();

    m_pLayer = CCLayer::create();
    addChild(m_pLayer);

    m_pButtonMenu = CCMenu::create();
    m_pButtonMenu->setPosition(winSize.width / 2, winSize.height / 2); // to center
    m_pLayer->addChild(m_pButtonMenu, 10);

    auto bg = extension::CCScale9Sprite::create("GJ_square04.png", {0, 0, 80, 80});
    bg->setContentSize(CCSize(400,280));
    m_pButtonMenu->addChild(bg);

    auto sprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(ReplayLayer::CloseCallback));
    button->setPosition(-195, 135);
    m_pButtonMenu->addChild(button);

    auto label = CCLabelBMFont::create("SC-Bot", "bigFont.fnt");
    label->setScale(0.8f);
    label->setPosition(0, 125);
    m_pButtonMenu->addChild(label);

    //Main Buttons

    auto btnsprite = extension::CCScale9Sprite::create("GJ_button_04.png", {0, 0, 40, 40});
    btnsprite->setContentSize(CCSize(55,25));

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onRecord));
    button->setPosition(-140 + (70*0), 80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Record", "goldFont.fnt");
    label->setScale(0.5f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onPlay));
    button->setPosition(-140 + (70*1), 80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Play", "goldFont.fnt");
    label->setScale(0.6f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onSave));
    button->setPosition(-140 + (70*2),80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Save", "goldFont.fnt");
    label->setScale(0.6f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onLoad));
    button->setPosition(-140 + (70*3),80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Load", "goldFont.fnt");
    label->setScale(0.6f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onRender));
    button->setPosition(-140 + (70*4), 80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Render", "goldFont.fnt");
    label->setScale(0.5f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);
    
    setTouchEnabled(true);
    setKeypadEnabled(true);
    return true;
}

void ReplayLayer::OpenCallback(CCObject* sender) {
    auto m_pButtonMenu = ReplayLayer::create();
    auto dir = CCDirector::sharedDirector();
    m_pButtonMenu->show();
    m_pButtonMenu->registerWithTouchDispatcher();
    dir->getTouchDispatcher()->incrementForcePrio(2);
}

void ReplayLayer::onRecord(CCObject*) {
    ReplaySystem::get().toggleRecording();
}

void ReplayLayer::onPlay(CCObject*) {
    ReplaySystem::get().togglePlaying();
}

void ReplayLayer::onSave(CCObject*) {}

void ReplayLayer::onLoad(CCObject*) {}

void ReplayLayer::onRender(CCObject*) {}