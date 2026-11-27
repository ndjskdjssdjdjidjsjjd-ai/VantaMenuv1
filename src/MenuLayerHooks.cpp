#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include "VantaState.hpp"
#include "VantaPopup.hpp"
#include "BgSelection1.hpp"
#include "Bg3DRenderer.hpp"

using namespace geode::prelude;

class $modify(VantaMenuLayer, MenuLayer) {
    struct Fields {
        CCMenu* m_vantaMenu = nullptr;
        CCMenuItemSpriteExtra* m_vantaBtn = nullptr;
        CCDrawNode* m_menuBgNode = nullptr;
        CCSprite* m_menuAnimSprite = nullptr;
        std::vector<SeasonParticle> m_menuParticles;
        float m_menuTime = 0.0f;
        bool m_draggingBtn = false;
        bool m_movedDuringDrag = false;
        CCPoint m_dragStartScreen = {0.f, 0.f};
        CCPoint m_btnStartPos = {0.f, 0.f};
    };

    void clampBtnPos(CCPoint& pos) {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        pos.x = std::clamp(pos.x, 42.f, std::max(42.f, winSize.width - 42.f));
        pos.y = std::clamp(pos.y, 24.f, std::max(24.f, winSize.height - 24.f));
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto& state = VantaState::get();
        state.initFeatures();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Main menu animation image & seasonal background
        m_fields->m_menuAnimSprite = CCSprite::create();
        m_fields->m_menuAnimSprite->setVisible(false);
        this->addChild(m_fields->m_menuAnimSprite, -2);

        m_fields->m_menuBgNode = CCDrawNode::create();
        this->addChild(m_fields->m_menuBgNode, -1);

        // Draggable + position-locked VANTA button
        m_fields->m_vantaMenu = CCMenu::create();
        CCPoint startPos = {54.f, winSize.height - 32.f};
        if (state.hasSavedVantaBtnPos) {
            startPos = {state.vantaBtnX, state.vantaBtnY};
            clampBtnPos(startPos);
        }
        m_fields->m_vantaMenu->setPosition(startPos);
        this->addChild(m_fields->m_vantaMenu, 50);

        auto spr = ButtonSprite::create(
            "VANTA", 62, true, "goldFont.fnt", "GJ_button_04.png", 26.f, 0.55f
        );
        m_fields->m_vantaBtn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(VantaMenuLayer::onVantaButtonClicked)
        );
        m_fields->m_vantaBtn->setPosition({0.f, 0.f});
        m_fields->m_vantaMenu->addChild(m_fields->m_vantaBtn);

        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);
        this->scheduleUpdate();
        return true;
    }

    void onVantaButtonClicked(CCObject*) {
        if (m_fields->m_movedDuringDrag) {
            m_fields->m_movedDuringDrag = false;
            return;
        }
        if (auto popup = VantaPopup::create()) {
            popup->show();
        }
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (m_fields->m_vantaMenu && m_fields->m_vantaBtn) {
            CCPoint local = m_fields->m_vantaMenu->convertTouchToNodeSpace(touch);
            CCSize sz = m_fields->m_vantaBtn->getContentSize();
            if (std::abs(local.x) <= sz.width * 0.65f && std::abs(local.y) <= sz.height * 0.65f) {
                m_fields->m_draggingBtn = true;
                m_fields->m_movedDuringDrag = false;
                m_fields->m_dragStartScreen = touch->getLocation();
                m_fields->m_btnStartPos = m_fields->m_vantaMenu->getPosition();
            }
        }
        return MenuLayer::ccTouchBegan(touch, event);
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        if (m_fields->m_draggingBtn && m_fields->m_vantaMenu) {
            CCPoint delta = touch->getLocation() - m_fields->m_dragStartScreen;
            if (delta.getLength() > 6.f) {
                m_fields->m_movedDuringDrag = true;
            }
            CCPoint nextPos = m_fields->m_btnStartPos + delta;
            clampBtnPos(nextPos);
            m_fields->m_vantaMenu->setPosition(nextPos);
        }
        MenuLayer::ccTouchMoved(touch, event);
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        if (m_fields->m_draggingBtn && m_fields->m_vantaMenu) {
            CCPoint finalPos = m_fields->m_vantaMenu->getPosition();
            clampBtnPos(finalPos);
            m_fields->m_vantaMenu->setPosition(finalPos);
            VantaState::get().setVantaBtnPos(finalPos.x, finalPos.y);
        }
        m_fields->m_draggingBtn = false;
        MenuLayer::ccTouchEnded(touch, event);
    }

    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
        m_fields->m_draggingBtn = false;
        MenuLayer::ccTouchCancelled(touch, event);
    }

    void update(float dt) override {
        MenuLayer::update(dt);
        m_fields->m_menuTime += dt;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        CCPoint camDrift = {
            std::sin(m_fields->m_menuTime * 0.45f) * 80.f,
            std::cos(m_fields->m_menuTime * 0.35f) * 35.f
        };

        Bg3DRenderer::renderBackground(
            m_fields->m_menuBgNode,
            m_fields->m_menuAnimSprite,
            winSize,
            camDrift,
            m_fields->m_menuTime,
            m_fields->m_menuParticles
        );
    }
};
