#include "ReplayLayer.h"
#include "ReplaySystem.h"
#include "RecorderLayer.h"
#include "Utils.h"
#include <nfd.h>
#include <shellapi.h>
#include "Version.h"

bool ReplayLayer::Init() {
    if(!initWithColor(ccc4(0, 0, 0, 130))) return false;

    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->getWinSize();
    auto& RS = ReplaySystem::get();

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

    auto label = CCLabelBMFont::create("Replay-Bot", "bigFont.fnt");
    label->setScale(0.8f);
    label->setPosition(0, 125);
    m_pButtonMenu->addChild(label);

    auto* const checkOffSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto* const checkOnSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

    //Main Buttons

    auto btnsprite = extension::CCScale9Sprite::create("GJ_button_04.png", {0, 0, 40, 40});
    btnsprite->setContentSize(CCSize(55,25));

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onRecord));
    button->setPosition(-105 + (70*0), 80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Record", "goldFont.fnt");
    label->setScale(0.5f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onPlay));
    button->setPosition(-105 + (70*1), 80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Play", "goldFont.fnt");
    label->setScale(0.6f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onSave));
    button->setPosition(-105 + (70*2),80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Save", "goldFont.fnt");
    label->setScale(0.6f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(ReplayLayer::onLoad));
    button->setPosition(-105 + (70*3),80);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Load", "goldFont.fnt");
    label->setScale(0.6f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);


    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(GetModuleHandleA(NULL), buffer, MAX_PATH);
    const auto path = std::filesystem::path(buffer).parent_path() / "ffmpeg.exe";
    if (std::filesystem::exists(path)) {
        button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(RecorderLayer::OpenCallback));
        button->setPosition(110, -114);
        m_pButtonMenu->addChild(button);
        label = CCLabelBMFont::create("Render", "goldFont.fnt");
        label->setScale(0.5f);
        label->setPosition(28, 14.5);
        label->setZOrder(1);
        button->addChild(label);
    }

    auto leftsprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    leftsprite->setScale(0.75f);
    auto rightsprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    rightsprite->setScale(0.75f);

    // FPS Input
    auto inputbg = extension::CCScale9Sprite::create("square02_small.png");
    inputbg->setContentSize(CCSize(64.f, 30.f));
    inputbg->setScale(1.f);
    inputbg->setOpacity(100);
    inputbg->setPosition(30, -20);
    m_pButtonMenu->addChild(inputbg);
    button = gd::CCMenuItemSpriteExtra::create(leftsprite, this, menu_selector(ReplayLayer::onModifyFps));
    button->setPosition(inputbg->getPositionX() - 40, inputbg->getPositionY());
    button->setTag(0);
    m_pButtonMenu->addChild(button);
    button = gd::CCMenuItemSpriteExtra::create(rightsprite, this, menu_selector(ReplayLayer::onModifyFps));
    button->setPosition(inputbg->getPositionX() + 40, inputbg->getPositionY());
    button->setTag(1);
    m_pButtonMenu->addChild(button);
    m_fpsInput = CCLabelBMFont::create("", "bigFont.fnt");
    m_fpsInput->setPosition(inputbg->getPositionX(), inputbg->getPositionY());
    m_fpsInput->setScale(0.7f);
    m_pButtonMenu->addChild(m_fpsInput);
    label = CCLabelBMFont::create("FPS", "bigFont.fnt");
    label->setScale(0.4f);
    label->setPosition(inputbg->getPositionX(), inputbg->getPositionY() + 25);
    m_pButtonMenu->addChild(label, 11);
    updateFpsInput();

    // Speed hack Input
    inputbg = extension::CCScale9Sprite::create("square02_small.png");
    inputbg->setContentSize(CCSize(64.f, 30.f));
    inputbg->setScale(1.f);
    inputbg->setOpacity(100);
    inputbg->setPosition(130, -20);
    m_pButtonMenu->addChild(inputbg);
    button = gd::CCMenuItemSpriteExtra::create(leftsprite, this, menu_selector(ReplayLayer::onModifySpdhk));
    button->setPosition(inputbg->getPositionX() - 40, inputbg->getPositionY());
    button->setTag(0);
    m_pButtonMenu->addChild(button);
    button = gd::CCMenuItemSpriteExtra::create(rightsprite, this, menu_selector(ReplayLayer::onModifySpdhk));
    button->setPosition(inputbg->getPositionX() + 40, inputbg->getPositionY());
    button->setTag(1);
    m_pButtonMenu->addChild(button);
    m_spdhkInput = CCLabelBMFont::create("", "bigFont.fnt");
    m_spdhkInput->setPosition(inputbg->getPositionX(), inputbg->getPositionY());
    m_spdhkInput->setScale(0.7f);
    m_pButtonMenu->addChild(m_spdhkInput);
    label = CCLabelBMFont::create("Speedhack", "bigFont.fnt");
    label->setScale(0.4f);
    label->setPosition(inputbg->getPositionX(), inputbg->getPositionY() + 25);
    m_pButtonMenu->addChild(label, 11);
    updateSpdhkInput();

    // auto record/save toggle
    auto* toggle = gd::CCMenuItemToggler::create(checkOffSprite, checkOnSprite, this, menu_selector(ReplayLayer::onToggleAutoRec));
    toggle->setPosition({-150, 35});
    toggle->setScale(0.8f);
    toggle->setSizeMult(1.2f);
    toggle->toggle(RS.isAutoRecording());
    m_pButtonMenu->addChild(toggle);
    label = CCLabelBMFont::create("Auto record/save", "bigFont.fnt");
    label->setAnchorPoint({0,0.5f});
    label->setScale(0.4f);
    label->setPosition({toggle->getPositionX() + 25, toggle->getPositionY()});
    m_pButtonMenu->addChild(label);

    // status label toggle
    toggle = gd::CCMenuItemToggler::create(checkOffSprite, checkOnSprite, this, menu_selector(ReplayLayer::onToggleStatusLabel));
    toggle->setPosition({-150, 5});
    toggle->setScale(0.8f);
    toggle->setSizeMult(1.2f);
    toggle->toggle(RS.isStatusLabel());
    m_pButtonMenu->addChild(toggle);
    label = CCLabelBMFont::create("Show status", "bigFont.fnt");
    label->setAnchorPoint({0,0.5f});
    label->setScale(0.4f);
    label->setPosition({toggle->getPositionX() + 25, toggle->getPositionY()});
    m_pButtonMenu->addChild(label);

    // real time toggle
    toggle = gd::CCMenuItemToggler::create(checkOffSprite, checkOnSprite, this, menu_selector(ReplayLayer::onToggleRealTime));
    toggle->setPosition({-150, -25});
    toggle->setScale(0.8f);
    toggle->setSizeMult(1.2f);
    toggle->toggle(RS.isLockDelta());
    m_pButtonMenu->addChild(toggle);
    label = CCLabelBMFont::create("Lock delta", "bigFont.fnt");
    label->setAnchorPoint({0,0.5f});
    label->setScale(0.4f);
    label->setPosition({toggle->getPositionX() + 25, toggle->getPositionY()});
    m_pButtonMenu->addChild(label);

    // practicefix toggle
    toggle = gd::CCMenuItemToggler::create(checkOffSprite, checkOnSprite, this, menu_selector(ReplayLayer::onTogglePracticeFix));
    toggle->setPosition({30, 35});
    toggle->setScale(0.8f);
    toggle->setSizeMult(1.2f);
    toggle->toggle(RS.isNoPracticeFix());
    m_pButtonMenu->addChild(toggle);
    label = CCLabelBMFont::create("No practice fix", "bigFont.fnt");
    label->setAnchorPoint({0,0.5f});
    label->setScale(0.4f);
    label->setPosition({toggle->getPositionX() + 25, toggle->getPositionY()});
    m_pButtonMenu->addChild(label);

    //ReplayInfo
    m_replayInfo = CCLabelBMFont::create("", "chatFont.fnt");
    m_replayInfo->setAnchorPoint({0, 1});
    m_replayInfo->setPosition(-190, -55);
    m_pButtonMenu->addChild(m_replayInfo);
    updateReplayInfo();

    //Replay folder location
    sprite = CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png");
    button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(ReplayLayer::onOpenRepFolder));
    button->setPosition(170, -115);
    m_pButtonMenu->addChild(button);

    //Bot Version label
    addChild(
        NodeFactory<CCLabelBMFont>::start(REPLAYBOT_VERSION, "chatFont.fnt")
        .setAnchorPoint(ccp(1, 0))
        .setScale(0.6f)
        .setPosition(ccp(winSize.width - 5, 5))
        .setOpacity(100u)
    );

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

void ReplayLayer::keyBackClicked() {
    auto& RS = ReplaySystem::get();
    RS.Save();
    FLAlertLayer::keyBackClicked();
}

void ReplayLayer::keyDown(enumKeyCodes key) {
    auto& RS = ReplaySystem::get();
    switch(key) {
        case 27: 
            keyBackClicked();
        break;
        case 188: 
            RS.modifySpeedhack(false); updateSpdhkInput();
        break;
        case 190: 
            RS.modifySpeedhack(true); updateSpdhkInput();
        break;
        case 'M':
            RS.setSpeedhack(1.f); updateSpdhkInput();
        break;
        case 37: 
            RS.modifyDefFps(false); updateFpsInput();
        break;
        case 39: 
            RS.modifyDefFps(true); updateFpsInput();
        break;
    }
}

void ReplayLayer::onRecord(CCObject*) {
    auto& RS = ReplaySystem::get();
    if(!RS.isRecording()){
        if(PlayLayer::get()) {
            if(RS.isPlaying()) {
                RS.toggleRecording();
                updateReplayInfo();
            } else {
                if(!RS.getReplay().getActions().empty()) {
                    showOverwriteAlert(100);
                } else {
                    RS.toggleRecording();
                    updateReplayInfo();
                }
            }
        } else {
            if(!RS.getReplay().getActions().empty()) {
                showOverwriteAlert(100);
            } else {
                RS.toggleRecording();
                updateReplayInfo();
            }
        }
    } else {
        RS.toggleRecording();
        updateReplayInfo();
    }
}

void ReplayLayer::onPlay(CCObject*) {
    ReplaySystem::get().togglePlaying();
    updateReplayInfo();
}

void ReplayLayer::onSave(CCObject*) {
    nfdchar_t* path = nullptr;
    nfdfilteritem_t filterItem[1] = {{"SC-Bot Replay", "screp"}};
    auto result = NFD_SaveDialog(&path, filterItem, 1, NULL, NULL);
    if(result == NFD_OKAY) {
        ReplaySystem::get().getReplay().Save(path);
        FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay saved.")->show();
        free(path);
    }
}

void ReplayLayer::onLoad(CCObject*) {
    auto& RS = ReplaySystem::get();
    if(RS.getReplay().getActions().empty()) {
        loadReplay();
    } else {
        showOverwriteAlert(101);
    }
}

void ReplayLayer::onToggleAutoRec(CCObject* sender) {
    auto toggle = cast<CCMenuItemToggler*>(sender);
    auto& RS = ReplaySystem::get();
    if(toggle != nullptr) {
        RS.toggleAutoRec();
    }
}

void ReplayLayer::onToggleStatusLabel(CCObject* sender) {
    auto toggle = cast<CCMenuItemToggler*>(sender);
    auto& RS = ReplaySystem::get();
    if(toggle != nullptr) {
        RS.toggleStatusLabel();
        RS.updateStatusLabel();
    }
}

void ReplayLayer::onToggleRealTime(CCObject* sender) {
    auto toggle = cast<CCMenuItemToggler*>(sender);
    auto& RS = ReplaySystem::get();
    if(toggle != nullptr) {
        RS.toggleRealTime();
    }
}

void ReplayLayer::onTogglePracticeFix(CCObject* sender) {
    auto toggle = cast<CCMenuItemToggler*>(sender);
    auto& RS = ReplaySystem::get();
    if(toggle != nullptr) {
        RS.togglePracticeFix();
    }
}

void ReplayLayer::onOpenRepFolder(CCObject*) {
    std::string folder = "SCReplays";
    if (!std::filesystem::is_directory(folder) || !std::filesystem::exists(folder))
        std::filesystem::create_directory(folder);
    ShellExecuteA(NULL, "open", "SCReplays\\", NULL, NULL, SW_SHOWDEFAULT);
}

void ReplayLayer::updateReplayInfo() {
    auto& RS = ReplaySystem::get();
    auto& replay = RS.getReplay();
    std::string replayStatus = "Stoped";
    std::string renderStatus = "";
    if(RS.isPlaying()) replayStatus = "Playing";
    if(RS.isRecording()) replayStatus = "Recording";
    if(RS.getRecorder().m_recording) renderStatus = " (Render)";
    std::stringstream stream;
    stream << "Status: " << replayStatus << renderStatus << "\n";
    stream << "Replay:\n";
    stream << "FPS: " << replay.getFps() << "\n";
    stream << "Actions: " << replay.getActions().size();
    m_replayInfo->setString(stream.str().c_str()); 
}

void ReplayLayer::updateFpsInput() {
    auto& RS = ReplaySystem::get();
    std::stringstream stream;
    stream << RS.getDefFps();
    m_fpsInput->setString(stream.str().c_str());
}

void ReplayLayer::updateSpdhkInput() {
    auto& RS = ReplaySystem::get();
    std::stringstream stream;
    stream << RS.getSpeedhack();
    m_spdhkInput->setString(stream.str().c_str());
}

void ReplayLayer::onModifyFps(CCObject* sender) {
    auto& RS = ReplaySystem::get();
    if(sender->getTag() == 1) RS.modifyDefFps(true);
    else RS.modifyDefFps(false);
    updateFpsInput();
}

void ReplayLayer::onModifySpdhk(CCObject* sender) {
    auto& RS = ReplaySystem::get();
    if(sender->getTag() == 1) RS.modifySpeedhack(true);
    else RS.modifySpeedhack(false);
    updateSpdhkInput();
}

void ReplayLayer::loadReplay() {
    nfdchar_t* path = nullptr;
    nfdfilteritem_t filterItem[1] = {{"SC-Mod Replay", "screp"}};
    auto result = NFD_OpenDialog(&path, filterItem, 1, NULL);
    if (result == NFD_OKAY) {
        auto& RS = ReplaySystem::get();
        RS.getReplay() = Replay::Load(path);
        if(!RS.isPlaying()) RS.togglePlaying();
        updateReplayInfo();
        FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay loaded.")->show();
        free(path);
    }
}
void ReplayLayer::showOverwriteAlert(int tag) {
    auto alert = FLAlertLayer::create(
        this,
        "Warning",
        "Ok",
        "Cancel",
        "This will <cr>overwrite</c> your current replay."
    );
    alert->setTag(tag);
    alert->show();
}

void ReplayLayer::FLAlert_Clicked(FLAlertLayer* alert, bool button2) {
    if(!button2) {
        //on record alert
        if(alert->getTag() == 100) {
            auto& RS = ReplaySystem::get();
            RS.toggleRecording();
            updateReplayInfo();
        }
        //on load alert
        if(alert->getTag() == 101) {
            loadReplay();
        }
    }
}