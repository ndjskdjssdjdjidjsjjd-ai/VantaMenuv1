#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>
#include <asp/fs/Path.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <optional>
#include "VantaState.hpp"

using namespace geode::prelude;

class VantaPopup : public geode::Popup {
protected:
    std::vector<std::string> m_categories = {
        "Player", "Engine", "Visuals", "Background", "HUD", "Practice", "Bypass"
    };
    std::string m_currentCategory = "Player";
    CCLayer* m_contentLayer = nullptr;
    CCClippingNode* m_clipper = nullptr;
    CCDrawNode* m_activeTabIndicator = nullptr;
    CCLabelBMFont* m_bgFileLabel = nullptr;
    CCSize m_popupSize = {456.f, 292.f};
    CCSize m_scrollSize = {296.f, 218.f};
    CCPoint m_clipOrigin = {0.f, 0.f};
    float m_maxScrollY = 0.f;
    float m_scrollY = 0.f;
    float m_lastTouchY = 0.f;
    CCPoint m_lastDragScreenPos = {0.f, 0.f};
    bool m_draggingScroll = false;
    bool m_draggingWindow = false;
    async::TaskHolder<Result<std::optional<asp::fs::Path>>> m_fileTaskHolder;

    bool init() override {
        if (!Popup::init(m_popupSize.width, m_popupSize.height)) return false;

        VantaState::get().initFeatures();
        m_currentCategory = VantaState::get().lastOpenedCategory;
        if (std::find(m_categories.begin(), m_categories.end(), m_currentCategory) == m_categories.end()) {
            m_currentCategory = "Player";
        }

        this->setTitle("VANTA MENU // KAIZXFAQ");
        auto winSize = m_mainLayer->getContentSize();

        auto accentBar = CCDrawNode::create();
        CCPoint barVerts[4] = {
            {18.f, winSize.height - 34.f},
            {winSize.width - 18.f, winSize.height - 34.f},
            {winSize.width - 18.f, winSize.height - 31.f},
            {18.f, winSize.height - 31.f}
        };
        accentBar->drawPolygon(barVerts, 4, {0.25f, 0.82f, 1.00f, 0.45f}, 0.f, {0.f, 0.f, 0.f, 0.f});
        m_mainLayer->addChild(accentBar);

        auto sidePanel = CCScale9Sprite::create("square02b_001.png");
        sidePanel->setColor({14, 18, 28});
        sidePanel->setOpacity(135);
        sidePanel->setContentSize({96.f, 232.f});
        sidePanel->setPosition({64.f, winSize.height * 0.5f - 16.f});
        m_mainLayer->addChild(sidePanel);

        m_activeTabIndicator = CCDrawNode::create();
        m_activeTabIndicator->setZOrder(5);
        m_mainLayer->addChild(m_activeTabIndicator);

        auto tabMenu = CCMenu::create();
        tabMenu->setPosition({64.f, winSize.height * 0.5f - 14.f});
        tabMenu->setZOrder(10);
        m_mainLayer->addChild(tabMenu);

        float startY = 88.f;
        float stepY = 29.5f;
        for (size_t i = 0; i < m_categories.size(); ++i) {
            auto cat = m_categories[i];
            auto spr = ButtonSprite::create(
                cat.c_str(), 82, true, "goldFont.fnt", "GJ_button_04.png", 22.f, 0.46f
            );
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(VantaPopup::onSelectTab)
            );
            btn->setID(cat);
            btn->setPosition({0.f, startY - (static_cast<float>(i) * stepY)});
            tabMenu->addChild(btn);
        }

        CCPoint listCenter = {winSize.width * 0.5f + 56.f, winSize.height * 0.5f - 14.f};

        auto listBg = CCScale9Sprite::create("square02b_001.png");
        listBg->setColor({10, 12, 20});
        listBg->setOpacity(140);
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

        auto footerLbl = CCLabelBMFont::create(
            "Drag Top Bar to Move | Drag List to Scroll | v2.2091", "chatFont.fnt"
        );
        footerLbl->setScale(0.50f);
        footerLbl->setOpacity(175);
        footerLbl->setPosition({winSize.width * 0.5f + 56.f, 13.f});
        m_mainLayer->addChild(footerLbl);

        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);
        updateActiveTabHighlight();
        populateCategory(m_currentCategory);
        return true;
    }

    void updateActiveTabHighlight() {
        if (!m_activeTabIndicator) return;
        m_activeTabIndicator->clear();

        int idx = 0;
        for (size_t i = 0; i < m_categories.size(); ++i) {
            if (m_categories[i] == m_currentCategory) {
                idx = static_cast<int>(i);
                break;
            }
        }
        auto winSize = m_mainLayer->getContentSize();
        float yCenter = (winSize.height * 0.5f - 14.f) + (88.f - idx * 29.5f);
        CCPoint pill[4] = {
            {18.f, yCenter - 10.f},
            {22.f, yCenter - 10.f},
            {22.f, yCenter + 10.f},
            {18.f, yCenter + 10.f}
        };
        m_activeTabIndicator->drawPolygon(pill, 4, {0.25f, 0.90f, 1.00f, 0.95f}, 0.f, {0.f, 0.f, 0.f, 0.f});
    }

    void clampPopupPosition() {
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float halfW = m_popupSize.width * 0.45f;
        float halfH = m_popupSize.height * 0.45f;
        CCPoint pos = this->getPosition();
        pos.x = std::clamp(pos.x, halfW, std::max(halfW, winSize.width - halfW));
        pos.y = std::clamp(pos.y, halfH, std::max(halfH, winSize.height - halfH));
        this->setPosition(pos);
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
        VantaState::get().savedTabScrollY[m_currentCategory] = m_scrollY;
    }

    bool isInsideScrollArea(CCTouch* touch) const {
        if (!m_mainLayer) return false;
        CCPoint local = m_mainLayer->convertTouchToNodeSpace(touch);
        return local.x >= m_clipOrigin.x &&
               local.x <= m_clipOrigin.x + m_scrollSize.width &&
               local.y >= m_clipOrigin.y &&
               local.y <= m_clipOrigin.y + m_scrollSize.height;
    }

    bool isInsideDragHeader(CCTouch* touch) const {
        if (!m_mainLayer) return false;
        CCPoint local = m_mainLayer->convertTouchToNodeSpace(touch);
        auto winSize = m_mainLayer->getContentSize();
        return local.x >= 40.f &&
               local.x <= winSize.width - 40.f &&
               local.y >= winSize.height - 42.f &&
               local.y <= winSize.height - 4.f;
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (isInsideDragHeader(touch)) {
            m_draggingWindow = true;
            m_lastDragScreenPos = touch->getLocation();
            return true;
        }
        if (isInsideScrollArea(touch) && m_maxScrollY > 0.f) {
            CCPoint local = m_mainLayer->convertTouchToNodeSpace(touch);
            m_draggingScroll = true;
            m_lastTouchY = local.y;
            return true;
        }
        return Popup::ccTouchBegan(touch, event);
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        if (m_draggingWindow) {
            CCPoint curScreen = touch->getLocation();
            CCPoint delta = curScreen - m_lastDragScreenPos;
            m_lastDragScreenPos = curScreen;
            this->setPosition(this->getPosition() + delta);
            clampPopupPosition();
            return;
        }
        if (m_draggingScroll) {
            CCPoint local = m_mainLayer->convertTouchToNodeSpace(touch);
            float deltaY = local.y - m_lastTouchY;
            m_lastTouchY = local.y;
            applyScrollLock(m_scrollY + deltaY);
            return;
        }
        Popup::ccTouchMoved(touch, event);
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        m_draggingWindow = false;
        m_draggingScroll = false;
        Popup::ccTouchEnded(touch, event);
    }

    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
        m_draggingWindow = false;
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
        VantaState::get().savedTabScrollY[m_currentCategory] = m_scrollY;
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        std::string nextCat = btn->getID();
        VantaState::get().setLastOpenedCategory(nextCat);
        updateActiveTabHighlight();
        populateCategory(nextCat);
    }

    void onChooseBackgroundPhoto(CCObject*) {
        utils::file::FilePickOptions options;
        options.filters.push_back({
            "Photo Files",
            {"*.png", "*.jpg", "*.jpeg", "*.webp"}
        });

        m_fileTaskHolder.spawn(
            utils::file::pick(utils::file::PickMode::OpenFile, options),
            [this](Result<std::optional<asp::fs::Path>> result) {
                if (result.isOk()) {
                    auto optPath = result.unwrap();
                    if (optPath.has_value()) {
                        std::string chosen = optPath.value().string();
                        auto& state = VantaState::get();
                        state.setCustomBgImagePath(chosen);
                        state.setBool("custom_bg_en", true);
                        if (m_currentCategory == "Background") {
                            populateCategory("Background");
                        }
                        FLAlertLayer::create("Custom Photo Set", chosen.c_str(), "OK")->show();
                    }
                }
            }
        );
    }

    void populateCategory(const std::string& category) {
        m_currentCategory = category;
        m_bgFileLabel = nullptr;
        m_contentLayer->removeAllChildren();

        auto& state = VantaState::get();
        std::vector<FeatureItem> filtered;
        for (const auto& f : state.features) {
            if (f.category == category) {
                filtered.push_back(f);
            }
        }

        bool isBgTab = (category == "Background");
        float extraHeader = isBgTab ? 42.f : 0.f;
        float itemHeight = 38.f;
        float totalHeight = std::max(
            m_scrollSize.height,
            extraHeader + static_cast<float>(filtered.size()) * itemHeight + 14.f
        );
        m_maxScrollY = std::max(0.f, totalHeight - m_scrollSize.height);
        m_contentLayer->setContentSize({m_scrollSize.width, totalHeight});

        float currentY = m_scrollSize.height - 20.f;

        if (isBgTab) {
            auto pickerRow = CCNode::create();
            pickerRow->setPosition({0.f, currentY});
            m_contentLayer->addChild(pickerRow);

            std::string pathPreview = state.customBgImagePath.empty()
                ? "No photo chosen yet"
                : state.customBgImagePath;
            if (pathPreview.size() > 30) {
                pathPreview = "..." + pathPreview.substr(pathPreview.size() - 27);
            }

            m_bgFileLabel = CCLabelBMFont::create(pathPreview.c_str(), "chatFont.fnt");
            m_bgFileLabel->setScale(0.42f);
            m_bgFileLabel->setColor({145, 225, 255});
            m_bgFileLabel->setAnchorPoint({0.f, 0.5f});
            m_bgFileLabel->setPosition({8.f, 0.f});
            pickerRow->addChild(m_bgFileLabel);

            auto rowMenu = CCMenu::create();
            rowMenu->setPosition({0.f, 0.f});
            pickerRow->addChild(rowMenu);

            auto pickSpr = ButtonSprite::create(
                "Choose File", 74, true, "bigFont.fnt", "GJ_button_01.png", 22.f, 0.38f
            );
            auto pickBtn = CCMenuItemSpriteExtra::create(
                pickSpr, this, menu_selector(VantaPopup::onChooseBackgroundPhoto)
            );
            pickBtn->setPosition({252.f, 0.f});
            rowMenu->addChild(pickBtn);

            currentY -= 38.f;
        }

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

        float restoredY = 0.f;
        auto it = state.savedTabScrollY.find(category);
        if (it != state.savedTabScrollY.end()) {
            restoredY = it->second;
        }
        applyScrollLock(restoredY);
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
