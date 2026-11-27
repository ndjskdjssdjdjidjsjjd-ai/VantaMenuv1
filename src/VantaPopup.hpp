#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "VantaState.hpp"

using namespace geode::prelude;

class VantaPopup : public geode::Popup {
protected:
    std::string m_currentCategory = "Player";
    CCLayer* m_contentLayer = nullptr;
    CCClippingNode* m_clipper = nullptr;
    CCSize m_scrollSize = {295.f, 215.f};
    CCPoint m_clipOrigin = {0.f, 0.f};
    float m_maxScrollY = 0.f;
    float m_scrollY = 0.f;
    float m_lastTouchY = 0.f;
    bool m_draggingScroll = false;

    bool init() override {
        if (!Popup::init(440.f, 285.f)) return false;

        this->setTitle("VantaMenuv1 // kaizxfaq");
        auto winSize = m_mainLayer->getContentSize();

        std::vector<std::string> categories = {
            "Player", "Engine", "Visuals", "HUD", "Practice", "Bypass"
        };
        auto tabMenu = CCMenu::create();
        tabMenu->setPosition({58.f, winSize.height / 2.f - 8.f});
        m_mainLayer->addChild(tabMenu);

        float startY = 85.f;
        for (size_t i = 0; i < categories.size(); ++i) {
            auto cat = categories[i];
            auto spr = ButtonSprite::create(
                cat.c_str(), 75, true, "goldFont.fnt", "GJ_button_04.png", 26.f, 0.54f
            );
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(VantaPopup::onSelectTab)
            );
            btn->setID(cat);
            btn->setPosition({0.f, startY - (static_cast<float>(i) * 34.f)});
            tabMenu->addChild(btn);
        }

        CCPoint listCenter = {winSize.width / 2.f + 50.f, winSize.height / 2.f - 10.f};

        auto listBg = CCScale9Sprite::create("square02b_001.png");
        listBg->setColor({0, 0, 0});
        listBg->setOpacity(110);
        listBg->setContentSize(m_scrollSize);
        listBg->setPosition(listCenter);
        m_mainLayer->addChild(listBg);

        auto stencil = CCDrawNode::create();
        CCPoint rectVerts[4] = {
            {0.f, 0.f},
            {0.f, m_scrollSize.height},
            {m_scrollSize.width, m_scrollSize.height},
            {m_scrollSize.width, 0.f}
        };
        stencil->drawPolygon(rectVerts, 4, {1.f, 1.f, 1.f, 1.f}, 0.f, {0.f, 0.f, 0.f, 0.f});

        m_clipOrigin = CCPoint{
            listCenter.x - (m_scrollSize.width * 0.5f),
            listCenter.y - (m_scrollSize.height * 0.5f)
        };

        m_clipper = CCClippingNode::create(stencil);
        m_clipper->setContentSize(m_scrollSize);
        m_clipper->setPosition(m_clipOrigin);
        m_mainLayer->addChild(m_clipper);

        m_contentLayer = CCLayer::create();
        m_contentLayer->setContentSize(m_scrollSize);
        m_clipper->addChild(m_contentLayer);

        auto countLabel = CCLabelBMFont::create(
            "100+ Modules | Drag Up/Down | v2.2091", "chatFont.fnt"
        );
        countLabel->setScale(0.55f);
        countLabel->setOpacity(160);
        countLabel->setPosition({winSize.width / 2.f + 50.f, 16.f});
        m_mainLayer->addChild(countLabel);

        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);
        populateCategory("Player");
        return true;
    }

    void applyScrollLock(float targetY) {
        if (m_maxScrollY <= 0.f) {
            m_scrollY = 0.f;
        } else if (targetY < 0.f) {
            m_scrollY = 0.f;
        } else if (targetY > m_maxScrollY) {
            m_scrollY = m_maxScrollY;
        } else {
            m_scrollY = targetY;
        }

        if (m_contentLayer) {
            m_contentLayer->setPositionY(m_scrollY);
        }
    }

    bool isInsideScrollArea(CCTouch* touch) const {
        if (!m_mainLayer) return false;
        CCPoint local = m_mainLayer->convertTouchToNodeSpace(touch);
        return local.x >= m_clipOrigin.x &&
               local.x <= m_clipOrigin.x + m_scrollSize.width &&
               local.y >= m_clipOrigin.y &&
               local.y <= m_clipOrigin.y + m_scrollSize.height;
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (isInsideScrollArea(touch) && m_maxScrollY > 0.f) {
            CCPoint local = m_mainLayer->convertTouchToNodeSpace(touch);
            m_draggingScroll = true;
            m_lastTouchY = local.y;
            return true;
        }
        return Popup::ccTouchBegan(touch, event);
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        if (m_draggingScroll) {
            CCPoint local = m_mainLayer->convertTouchToNodeSpace(touch);
            float deltaY = local.y - m_lastTouchY;
            m_lastTouchY = local.y;
            // Finger moving down (deltaY < 0) decreases m_scrollY -> scrolls back up toward top (0.f)
            // Finger moving up (deltaY > 0) increases m_scrollY -> scrolls down toward bottom (m_maxScrollY)
            applyScrollLock(m_scrollY + deltaY);
            return;
        }
        Popup::ccTouchMoved(touch, event);
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        m_draggingScroll = false;
        Popup::ccTouchEnded(touch, event);
    }

    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
        m_draggingScroll = false;
        Popup::ccTouchCancelled(touch, event);
    }

    std::string formatSliderValue(const FeatureItem& feat, float val) {
        if (!feat.choices.empty()) {
            int idx = std::clamp(
                static_cast<int>(std::round(val)),
                0,
                static_cast<int>(feat.choices.size()) - 1
            );
            return feat.choices[idx];
        }
        return fmt::format("{:.2f}", val);
    }

    void onSelectTab(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        populateCategory(btn->getID());
    }

    void populateCategory(const std::string& category) {
        m_currentCategory = category;
        m_contentLayer->removeAllChildren();

        auto& state = VantaState::get();
        std::vector<FeatureItem> filtered;
        for (const auto& f : state.features) {
            if (f.category == category) {
                filtered.push_back(f);
            }
        }

        float itemHeight = 38.f;
        float totalHeight = std::max(
            m_scrollSize.height,
            static_cast<float>(filtered.size()) * itemHeight + 12.f
        );
        m_maxScrollY = std::max(0.f, totalHeight - m_scrollSize.height);
        m_contentLayer->setContentSize({m_scrollSize.width, totalHeight});

        applyScrollLock(0.f);

        float currentY = m_scrollSize.height - 22.f;

        for (const auto& feat : filtered) {
            auto row = CCNode::create();
            row->setPosition({0.f, currentY});
            m_contentLayer->addChild(row);

            auto nameLbl = CCLabelBMFont::create(feat.name.c_str(), "bigFont.fnt");
            nameLbl->setScale(0.36f);
            nameLbl->setAnchorPoint({0.f, 0.5f});
            nameLbl->setPosition({8.f, 6.f});
            row->addChild(nameLbl);

            auto descLbl = CCLabelBMFont::create(feat.desc.c_str(), "chatFont.fnt");
            descLbl->setScale(0.38f);
            descLbl->setColor({180, 180, 180});
            descLbl->setAnchorPoint({0.f, 0.5f});
            descLbl->setPosition({8.f, -8.f});
            row->addChild(descLbl);

            auto rowMenu = CCMenu::create();
            rowMenu->setPosition({0.f, 0.f});
            row->addChild(rowMenu);

            if (!feat.isSlider) {
                auto toggle = CCMenuItemToggler::createWithStandardSprites(
                    this, menu_selector(VantaPopup::onToggleFeature), 0.6f
                );
                toggle->setID(feat.id);
                toggle->toggle(state.getBool(feat.id));
                toggle->setPosition({270.f, 0.f});
                rowMenu->addChild(toggle);
            } else {
                float val = state.getFloat(feat.id);
                auto valStr = formatSliderValue(feat, val);
                auto valLbl = CCLabelBMFont::create(valStr.c_str(), "goldFont.fnt");
                valLbl->setScale(feat.choices.empty() ? 0.4f : 0.32f);
                valLbl->setID(feat.id + "_lbl");
                valLbl->setPosition({232.f, 0.f});
                row->addChild(valLbl);

                auto minusSpr = ButtonSprite::create(
                    "-", 18, true, "bigFont.fnt", "GJ_button_06.png", 18.f, 0.45f
                );
                auto minusBtn = CCMenuItemSpriteExtra::create(
                    minusSpr, this, menu_selector(VantaPopup::onSliderStep)
                );
                minusBtn->setID(feat.id + "|down");
                minusBtn->setPosition({188.f, 0.f});
                rowMenu->addChild(minusBtn);

                auto plusSpr = ButtonSprite::create(
                    "+", 18, true, "bigFont.fnt", "GJ_button_01.png", 18.f, 0.45f
                );
                auto plusBtn = CCMenuItemSpriteExtra::create(
                    plusSpr, this, menu_selector(VantaPopup::onSliderStep)
                );
                plusBtn->setID(feat.id + "|up");
                plusBtn->setPosition({276.f, 0.f});
                rowMenu->addChild(plusBtn);
            }

            currentY -= itemHeight;
        }
    }

    void onToggleFeature(CCObject* sender) {
        auto toggle = static_cast<CCMenuItemToggler*>(sender);
        std::string id = toggle->getID();
        bool nextState = !toggle->isToggled();
        VantaState::get().setBool(id, nextState);

        if (id == "fps_bypass_en" || id == "tps_bypass_en") {
            applyFrameRates();
        }
    }

    void onSliderStep(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        std::string raw = btn->getID();
        size_t sep = raw.find('|');
        if (sep == std::string::npos) return;

        std::string id = raw.substr(0, sep);
        std::string dir = raw.substr(sep + 1);

        auto& state = VantaState::get();
        for (const auto& f : state.features) {
            if (f.id == id) {
                float step = !f.choices.empty() ? 1.0f : ((f.maxVal - f.minVal) / 20.0f);
                if (id == "fps_val" || id == "tps_val") step = 15.0f;
                float cur = state.getFloat(id);
                cur += (dir == "up" ? step : -step);
                cur = std::clamp(cur, f.minVal, f.maxVal);
                if (!f.choices.empty()) {
                    cur = std::round(cur);
                }
                state.setFloat(id, cur);

                if (auto lbl = typeinfo_cast<CCLabelBMFont*>(
                    m_contentLayer->getChildByIDRecursive(id + "_lbl")
                )) {
                    lbl->setString(formatSliderValue(f, cur).c_str());
                }
                if (id == "fps_val" || id == "tps_val") applyFrameRates();
                break;
            }
        }
    }

public:
    static void applyFrameRates() {
        auto& state = VantaState::get();
        if (state.getBool("fps_bypass_en")) {
            float fps = state.getFloat("fps_val");
            CCDirector::sharedDirector()->setAnimationInterval(1.0 / fps);
        }
    }

    static VantaPopup* create() {
        auto ret = new VantaPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
