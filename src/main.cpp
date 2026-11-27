#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "VantaState.hpp"
#include "VantaPopup.hpp"

using namespace geode::prelude;

class $modify(VantaMainMenu, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        VantaState::get().initFeatures();

        if (!VantaState::get().getBool("hide_menu_btn")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto menu = CCMenu::create();
            menu->setPosition({35.f, winSize.height - 35.f});
            menu->setID("vanta-main-menu");

            auto spr = ButtonSprite::create(
                "Vanta", 45, true, "goldFont.fnt", "GJ_button_05.png", 28.f, 0.6f
            );
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(VantaMainMenu::onOpenVanta)
            );
            menu->addChild(btn);
            this->addChild(menu, 100);
        }

        return true;
    }

    void onOpenVanta(CCObject*) {
        if (auto popup = VantaPopup::create()) {
            popup->show();
        }
    }
};

class $modify(VantaPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        VantaState::get().initFeatures();

        if (!VantaState::get().getBool("hide_menu_btn")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            auto menu = CCMenu::create();
            menu->setPosition({45.f, winSize.height - 35.f});
            auto spr = ButtonSprite::create(
                "Vanta", 45, true, "goldFont.fnt", "GJ_button_05.png", 28.f, 0.6f
            );
            auto btn = CCMenuItemSpriteExtra::create(
                spr, this, menu_selector(VantaPauseLayer::onOpenVantaPause)
            );
            menu->addChild(btn);
            this->addChild(menu, 100);
        }
    }

    void onOpenVantaPause(CCObject*) {
        if (auto popup = VantaPopup::create()) {
            popup->show();
        }
    }
};

class $modify(VantaGameManager, GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        if (VantaState::get().getBool("icon_hack")) return true;
        return GameManager::isIconUnlocked(id, type);
    }

    bool isColorUnlocked(int id, UnlockType type) {
        if (VantaState::get().getBool("color_hack")) return true;
        return GameManager::isColorUnlocked(id, type);
    }
};

class $modify(VantaLevelInfo, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (VantaState::get().getBool("copy_hack")) {
            level->m_password = 1;
        }
        return LevelInfoLayer::init(level, challenge);
    }
};
