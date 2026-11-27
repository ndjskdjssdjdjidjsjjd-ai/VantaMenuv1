#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include "VantaState.hpp"

using namespace geode::prelude;

class VantaPopup : public geode::Popup {
protected:
    std::string m_currentCategory = "Player";
    CCLayer* m_contentLayer = nullptr;
    CCSize m_scrollSize = {290.f, 210.f};
    float m_maxScrollY = 0.f;
    float m_scrollY = 0.f;

    bool init() {
        if (!Popup::init(440.f, 285.f)) return false;

        this->setTitle("VantaMenuv1 // kaizxfaq");
        auto winSize = m_mainLayer->getContentSize();

        std::vector<std::string> categories = {
            "Player", "Engine", "Visuals", "HUD", "Practice", "Bypass", "Settings"
        };
        auto tabMenu = CCMenu::create();
        tabMenu->setPosition({58.f, winSize.height / 2.f - 6.f});
        m_mainLayer->addChild(tabMenu);

        float startY = 96.f;
        for (size_t i = 0; i < categories.size(); ++i) {
            auto cat = categories[i];
            auto spr = ButtonSprite::create(
                cat.c_str(), 75, true, "goldFont.fnt", "GJ_button_04.png", 24.f, 0.52f
            );
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(VantaPopup::onSelectTab)
            );
            btn->setID(cat);
            btn->setPosition({0.f, startY - (static_cast<float>(i) * 31.f)});
            tabMenu->addChild(btn);
        }

        CCPoint listCenter = {winSize.width / 2.f + 48.f, winSize.height / 2.f - 10.f};
        CCSize boxSize = {295.f, 215.f};

        auto listBg = CCScale9Sprite::create("square02b_001.png");
        listBg->setColor({0, 0, 0});
        listBg->setOpacity(110);
        listBg->setContentSize(boxSize);
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

        auto clipper = CCClippingNode::create(stencil);
        clipper->setContentSize(m_scrollSize);
        clipper->setPosition({
            listCenter.x - m_scrollSize.width / 2.f,
            listCenter.y - m_scrollSize.height / 2.f
        });
        m_mainLayer->addChild(clipper);

        m_contentLayer = CCLayer::create();
        m_contentLayer->setContentSize(m_scrollSize);
        clipper->addChild(m_contentLayer);

        // Quick scroll up/down buttons on the right edge for easy mobile/mouse scrolling
        auto scrollMenu = CCMenu::create();
        scrollMenu->setPosition({listCenter.x + boxSize.width / 2.f + 12.f, listCenter.y});
        m_mainLayer->addChild(scrollMenu);

        auto upSpr = ButtonSprite::create("^", 16, true, "bigFont.fnt", "GJ_button_04.png", 18.f, 0.45f);
        auto upBtn = CCMenuItemSpriteExtra::create(upSpr, this, menu_selector(VantaPopup::onScrollUp));
        upBtn->setPosition({0.f, 45.f});
        scrollMenu->addChild(upBtn);

        auto dnSpr = ButtonSprite::create("v", 16, true, "bigFont.fnt", "GJ_button_04.png", 18.f, 0.45f);
        auto dnBtn = CCMenuItemSpriteExtra::create(dnSpr, this, menu_selector(VantaPopup::onScrollDown));
        dnBtn->setPosition({0.f, -45.f});
        scrollMenu->addChild(dnBtn);

        auto countLabel = CCLabelBMFont::create(
            "100 Modules + Settings | Multi-Platform v2.2091", "chatFont.fnt"
        );
        countLabel->setScale(0.55f);
        countLabel->setOpacity(160);
        countLabel->setPosition({winSize.width / 2.f + 48.f, 16.f});
        m_mainLayer->addChild(countLabel);

        populateCategory("Player");
        return true;
    }

    void onSelectTab(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        populateCategory(btn->getID());
    }

    void onScrollUp(CCObject*) {
        m_scrollY = std::max(0.f, m_scrollY - 76.f);
        m_contentLayer->setPositionY(m_scrollY);
    }

    void onScrollDown(CCObject*) {
        m_scrollY = std::min(m_maxScrollY, m_scrollY + 76.f);
        m_contentLayer->setPositionY(m_scrollY);
    }

    void populateCategory(const std::string& category) {
        m_currentCategory = category;
        m_contentLayer->removeAllChildren();
        m_scrollY = 0.f;
        m_contentLayer->setPositionY(0.f);

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

        float currentY = totalHeight - 22.f;

        for (const auto& feat : filtered) {
            auto row = CCNode::create();
            row->setPosition({0.f, currentY});
            m_contentLayer->addChild(row);

            auto nameLbl = CCLabelBMFont::create(feat.name.c_str(), "bigFont.fnt");
            nameLbl->setScale(0.38f);
            nameLbl->setAnchorPoint({0.f, 0.5f});
            nameLbl->setPosition({8.f, 6.f});
            row->addChild(nameLbl);

            auto descLbl = CCLabelBMFont::create(feat.desc.c_str(), "chatFont.fnt");
            descLbl->setScale(0.4f);
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
                toggle->setPosition({266.f, 0.f});
                rowMenu->addChild(toggle);
            } else {
                float val = state.getFloat(feat.id);
                auto valStr = fmt::format("{:.2f}", val);
                auto valLbl = CCLabelBMFont::create(valStr.c_str(), "goldFont.fnt");
                valLbl->setScale(0.42f);
                valLbl->setID(feat.id + "_lbl");
                valLbl->setPosition({240.f, 0.f});
                row->addChild(valLbl);

                auto minusSpr = ButtonSprite::create(
                    "-", 18, true, "bigFont.fnt", "GJ_button_06.png", 18.f, 0.45f
                );
                auto minusBtn = CCMenuItemSpriteExtra::create(
                    minusSpr, this, menu_selector(VantaPopup::onSliderStep)
                );
                minusBtn->setID(feat.id + "|down");
                minusBtn->setPosition({208.f, 0.f});
                rowMenu->addChild(minusBtn);

                auto plusSpr = ButtonSprite::create(
                    "+", 18, true, "bigFont.fnt", "GJ_button_01.png", 18.f, 0.45f
                );
                auto plusBtn = CCMenuItemSpriteExtra::create(
                    plusSpr, this, menu_selector(VantaPopup::onSliderStep)
                );
                plusBtn->setID(feat.id + "|up");
                plusBtn->setPosition({272.f, 0.f});
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
                float step = (f.maxVal - f.minVal) / 20.0f;
                if (id == "fps_val" || id == "tps_val") step = 15.0f;
                float cur = state.getFloat(id);
                cur += (dir == "up" ? step : -step);
                cur = std::clamp(cur, f.minVal, f.maxVal);
                state.setFloat(id, cur);

                if (auto lbl = typeinfo_cast<CCLabelBMFont*>(
                    m_contentLayer->getChildByIDRecursive(id + "_lbl")
                )) {
                    lbl->setString(fmt::format("{:.2f}", cur).c_str());
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
