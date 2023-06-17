#include "ReplayLayer.h"
#include "ReplaySystem.h"
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

    auto label = CCLabelBMFont::create("SC-Bot", "bigFont.fnt");
    label->setScale(0.8f);
    label->setPosition(0, 125);
    m_pButtonMenu->addChild(label);

    auto* const checkOffSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto* const checkOnSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

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

    // FPS Input
    m_fpsInput = NumberInputNode::create(CCSize(64.f, 30.f));
    m_fpsInput->set_value(static_cast<int>(RS.getDefFps()));
    m_fpsInput->input_node->setMaxLabelScale(0.7f);
    m_fpsInput->input_node->setMaxLabelLength(3);
    m_fpsInput->setPosition((winSize.width / 2) - 125,(winSize.height / 2) + 30);
    m_fpsInput->callback = [&](auto& m_fpsInput) {
        RS.setDefFps(static_cast<int>(m_fpsInput.get_value()));
    };
    m_pLayer->addChild(m_fpsInput, 11);
    label = CCLabelBMFont::create("FPS", "bigFont.fnt");
    label->setScale(0.4f);
    label->setPosition({-125, 53});
    m_pButtonMenu->addChild(label);

    // auto record/save toggle
    auto* toggle = gd::CCMenuItemToggler::create(checkOffSprite, checkOnSprite, this, menu_selector(ReplayLayer::onToggleAutoRec));
    toggle->setPosition({-140, -20});
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
    toggle->setPosition({45, -20});
    toggle->setScale(0.8f);
    toggle->setSizeMult(1.2f);
    toggle->toggle(RS.isStatusLabel());
    m_pButtonMenu->addChild(toggle);
    label = CCLabelBMFont::create("Show Status Label", "bigFont.fnt");
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


void ReplayLayer::onRecord(CCObject*) {
    auto& RS = ReplaySystem::get();
    if(!RS.isRecording()){
        if(RS.getReplay().getActions().empty()) {
            RS.toggleRecording();
            updateReplayInfo();
        } else {
            auto alert = FLAlertLayer::create(
                this,
                "Warning",
                "Ok",
                "Cancel",
                "This will <cr>overwrite</c> your current replay."
            );
            alert->setTag(100);
            alert->show();
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
        auto alert = FLAlertLayer::create(
            this,
            "Warning",
            "Ok",
            "Cancel",
            "This will <cr>overwrite</c> your current replay."
        );
        alert->setTag(101);
        alert->show(); 
    }
}

void ReplayLayer::onRender(CCObject*) {}

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
    if(RS.isPlaying()) replayStatus = "Playing";
    if(RS.isRecording()) replayStatus = "Recording";
    std::stringstream stream;
    stream << "Status: " << replayStatus << "\n";
    stream << "Replay:\n";
    stream << "FPS: " << replay.getFps() << "\n";
    stream << "Actions: " << replay.getActions().size();
    m_replayInfo->setString(stream.str().c_str()); 
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