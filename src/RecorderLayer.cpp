#include "RecorderLayer.h"
#include "ReplaySystem.h"
#include "ReplayLayer.h"
#include <nfd.h>

bool RecorderLayer::Init() {
    if(!initWithColor(ccc4(0, 0, 0, 130))) return false;

    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->getWinSize();
    auto& RS = ReplaySystem::get();

    m_pLayer = CCLayer::create();
    addChild(m_pLayer);

    m_pButtonMenu = CCMenu::create();
    m_pButtonMenu->setPosition(winSize.width / 2, winSize.height / 2);
    m_pLayer->addChild(m_pButtonMenu, 10);

    auto bg = extension::CCScale9Sprite::create("GJ_square04.png", {0, 0, 80, 80});
    bg->setContentSize(CCSize(375,255));
    m_pButtonMenu->addChild(bg);

    auto sprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(RecorderLayer::CloseCallback));
    button->setPosition(-182.5, 122.5);
    m_pButtonMenu->addChild(button);

    auto label = CCLabelBMFont::create("Render Level", "bigFont.fnt");
    label->setScale(0.8f);
    label->setPosition(0, 112.5);
    m_pButtonMenu->addChild(label);

    auto* const checkOffSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto* const checkOnSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto btnsprite = extension::CCScale9Sprite::create("GJ_button_04.png", {0, 0, 40, 40});
    btnsprite->setContentSize(CCSize(55,25));
    const std::string broadFilter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,;-_=+@!'\":0123456789$[](){} ";


    // Render Button
    button = gd::CCMenuItemSpriteExtra::create(btnsprite, this, menu_selector(RecorderLayer::onRender));
    button->setPosition(0, 75);
    button->setScale(1.3f);
    m_pButtonMenu->addChild(button);
    label = CCLabelBMFont::create("Start", "goldFont.fnt");
    label->setScale(0.5f);
    label->setPosition(28, 14.5);
    label->setZOrder(1);
    button->addChild(label);

    //Toggle render until end
    auto* toggle = gd::CCMenuItemToggler::create(checkOffSprite, checkOnSprite, this, menu_selector(RecorderLayer::onToggleUntilEnd));
    toggle->setPosition({-115, -10});
    toggle->setScale(0.8f);
    toggle->setSizeMult(1.2f);
    toggle->toggle(RS.getRecorder().m_untilEnd);
    m_pButtonMenu->addChild(toggle);
    label = CCLabelBMFont::create("Render until the end", "bigFont.fnt");
    label->setAnchorPoint({0,0.5f});
    label->setScale(0.4f);
    label->setPosition({toggle->getPositionX() + 25, toggle->getPositionY()});
    m_pButtonMenu->addChild(label);

    //Toggle include audio
    toggle = gd::CCMenuItemToggler::create(checkOffSprite, checkOnSprite, this, menu_selector(RecorderLayer::onToggleIncludeAudio));
    toggle->setPosition({-55, -45});
    toggle->setScale(0.8f);
    toggle->setSizeMult(1.2f);
    toggle->toggle(RS.getRecorder().m_includeAudio);
    m_pButtonMenu->addChild(toggle);
    label = CCLabelBMFont::create("Include audio", "bigFont.fnt");
    label->setAnchorPoint({0,0.5f});
    label->setScale(0.4f);
    label->setPosition({toggle->getPositionX() + 25, toggle->getPositionY()});
    m_pButtonMenu->addChild(label);

    //set actual win size
    sprite = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(RecorderLayer::getWinSize));
    button->setPosition(-165, 30);
    m_pButtonMenu->addChild(button);

    //resolution input position
    float xpos = (winSize.width / 2) - 120;
    float ypos = (winSize.height / 2) + 30;

    // width input
    m_widthInput = NumberInputNode::create(CCSize(64.f, 30.f));
    m_widthInput->set_value(static_cast<int>(RS.getRecorder().m_width));
    m_widthInput->input_node->setMaxLabelScale(0.7f);
    m_widthInput->input_node->setMaxLabelLength(3);
    m_widthInput->setPosition(xpos,ypos);
    m_widthInput->callback = [&](auto& m_widthInput) {
        RS.getRecorder().m_width = m_widthInput.get_value();
    };
    m_pLayer->addChild(m_widthInput, 11);
    label = CCLabelBMFont::create("x", "bigFont.fnt");
    label->setScale(0.4f);
    label->setPosition({m_widthInput->getPositionX() + 40, m_widthInput->getPositionY()});
    m_pLayer->addChild(label, 11);

    // height input
    m_heightInput = NumberInputNode::create(CCSize(64.f, 30.f));
    m_heightInput->set_value(static_cast<int>(RS.getRecorder().m_height));
    m_heightInput->input_node->setMaxLabelScale(0.7f);
    m_heightInput->input_node->setMaxLabelLength(3);
    m_heightInput->setPosition(xpos + (80*1),ypos);
    m_heightInput->callback = [&](auto& m_heightInput) {
        RS.getRecorder().m_height = m_heightInput.get_value();
    };
    m_pLayer->addChild(m_heightInput, 11);
    label = CCLabelBMFont::create("@", "bigFont.fnt");
    label->setScale(0.4f);
    label->setPosition({m_heightInput->getPositionX() + 40, m_heightInput->getPositionY()});
    m_pLayer->addChild(label, 11);

    // fps input
    auto input = NumberInputNode::create(CCSize(64.f, 30.f));
    input->set_value(static_cast<int>(RS.getRecorder().m_fps));
    input->input_node->setMaxLabelScale(0.7f);
    input->input_node->setMaxLabelLength(3);
    input->setPosition(xpos + (80*2),ypos);
    input->callback = [&](auto& input) {
        RS.getRecorder().m_fps = input.get_value();
    };
    m_pLayer->addChild(input, 11);
    label = CCLabelBMFont::create("br", "bigFont.fnt");
    label->setScale(0.4f);
    label->setPosition({input->getPositionX() + 41.5f, input->getPositionY()});
    m_pLayer->addChild(label, 11);

    //bitrate input
    auto textInput = TextInputNode::create(CCSize(60.f, 30.f), 1.f, "chatFont.fnt");
    textInput->setPosition(xpos + (80*3),  ypos);
    textInput->input_node->setAllowedChars(broadFilter);
    textInput->set_value(RS.getRecorder().m_bitrate);
    textInput->callback = [&RS](auto& input) {
        RS.getRecorder().m_bitrate = input.get_value();
    };
    m_pLayer->addChild(textInput, 11);

    // end duration input
    input = NumberInputNode::create(CCSize(64.f, 30.f));
    input->set_value(static_cast<int>(RS.getRecorder().m_afterEndDuration));
    input->input_node->setMaxLabelScale(0.7f);
    input->input_node->setMaxLabelLength(3);
    input->setPosition((winSize.width / 2) + 100,  (winSize.height / 2) - 10);
    input->callback = [&](auto& input) {
        RS.getRecorder().m_afterEndDuration = static_cast<float>(input.get_value());
    };
    m_pLayer->addChild(input, 11);

    //codec input
    textInput = TextInputNode::create(CCSize(60.f, 30.f), 1.f, "chatFont.fnt");
    textInput->setPosition((winSize.width / 2) - 80,  (winSize.height / 2) - 90);
    textInput->input_node->setAllowedChars(broadFilter);
    textInput->input_node->m_sCaption = "Codec";
    textInput->input_node->setLabelPlaceholderColor({200, 200, 200});
    textInput->set_value(RS.getRecorder().m_codec);
    textInput->callback = [&RS](auto& input) {
        RS.getRecorder().m_codec = input.get_value();
    };
    m_pLayer->addChild(textInput, 11);

    //extra args input
    textInput = TextInputNode::create(CCSize(60.f, 30.f), 1.f, "chatFont.fnt");
    textInput->setPosition((winSize.width / 2),  (winSize.height / 2) - 90);
    textInput->input_node->setAllowedChars(broadFilter);
    textInput->input_node->m_sCaption = "Extra args";
    textInput->input_node->setLabelPlaceholderColor({200, 200, 200});
    textInput->set_value(RS.getRecorder().m_extraArgs);
    textInput->callback = [&RS](auto& input) {
        RS.getRecorder().m_extraArgs = input.get_value();
    };
    m_pLayer->addChild(textInput, 11);

    //extra audio args input
    textInput = TextInputNode::create(CCSize(60.f, 30.f), 1.f, "chatFont.fnt");
    textInput->setPosition((winSize.width / 2) + 80,  (winSize.height / 2) - 90);
    textInput->input_node->setAllowedChars(broadFilter);
    textInput->input_node->m_sCaption = "Audio args";
    textInput->input_node->setLabelPlaceholderColor({200, 200, 200});
    textInput->set_value(RS.getRecorder().m_extraAudioArgs);
    textInput->callback = [&RS](auto& input) {
        RS.getRecorder().m_extraAudioArgs = input.get_value();
    };
    m_pLayer->addChild(textInput, 11);

    setTouchEnabled(true);
    setKeypadEnabled(true);
    return true;
}

void RecorderLayer::OpenCallback(CCObject*) {
    auto m_pButtonMenu = RecorderLayer::create();
    auto dir = CCDirector::sharedDirector();
    m_pButtonMenu->show();
    m_pButtonMenu->registerWithTouchDispatcher();
    dir->getTouchDispatcher()->incrementForcePrio(2);
}

void RecorderLayer::keyBackClicked() {
    FLAlertLayer::keyBackClicked();
}

void RecorderLayer::onRender(CCObject*) {
    auto& RS = ReplaySystem::get();
    if(RS.getRecorder().m_recording) {
        if(PlayLayer::get()) RS.getRecorder().Stop();
        RS.getRecorder().m_recording = false;
        std::cout << "render stoped" << std::endl;
    } else {
        nfdchar_t* path = nullptr;
        nfdfilteritem_t filterItem[1] = {{"Video", "mp4,mkv,webm"}};
        auto result = NFD_SaveDialog(&path, filterItem, 1, NULL, NULL);
        if(result == NFD_OKAY) {
            //if(!RS.isPlaying()) RS.togglePlaying();
            std::cout << "save video to: " << path << std::endl;
            RS.getRecorder().m_recording = true;
            RS.getRecorder().videoPath = path;
            keyBackClicked();
            free(path);
        }
    }
}

void RecorderLayer::onToggleUntilEnd(CCObject* obj) {
    ReplaySystem::get().getRecorder().m_untilEnd = !static_cast<CCMenuItemToggler*>(obj)->isOn();
}

void RecorderLayer::onToggleIncludeAudio(CCObject* obj) {
    ReplaySystem::get().getRecorder().m_includeAudio = !static_cast<CCMenuItemToggler*>(obj)->isOn();
}

void RecorderLayer::getWinSize(CCObject*) {
    auto dir = CCDirector::sharedDirector();
    auto winSize = dir->m_obResolutionInPixels;
    auto& RS = ReplaySystem::get();

    m_widthInput->set_value(static_cast<int>(winSize.width));
    m_heightInput->set_value(static_cast<int>(winSize.height));
    RS.getRecorder().m_width = static_cast<int>(winSize.width);
    RS.getRecorder().m_height = static_cast<int>(winSize.height);
}