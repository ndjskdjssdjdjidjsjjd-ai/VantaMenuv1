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
    struct Fields {
        GJGameLevel* m_levelRef = nullptr;
    };

    bool init(GJGameLevel* level, bool challenge) {
        m_fields->m_levelRef = level;
        auto& state = VantaState::get();

        if (level && state.getBool("copy_hack")) {
            level->m_password = 1;
        }

        bool ok = LevelInfoLayer::init(level, challenge);
        if (!ok) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto utilMenu = CCMenu::create();
        utilMenu->setPosition({winSize.width - 62.f, winSize.height - 36.f});
        utilMenu->setZOrder(120);
        this->addChild(utilMenu);

        auto copySpr = ButtonSprite::create(
            "Copy Lvl", 56, true, "bigFont.fnt", "GJ_button_04.png", 22.f, 0.42f
        );
        auto copyBtn = CCMenuItemSpriteExtra::create(
            copySpr, this, menu_selector(VantaLevelInfo::onCopyLevelNow)
        );
        copyBtn->setPosition({0.f, 0.f});
        utilMenu->addChild(copyBtn);

        // Auto Song Download on level info load
        if (level && state.getBool("auto_song_download") && level->m_songID > 0) {
            if (m_downloadSongBtn && m_downloadSongBtn->isVisible()) {
                this->onDownloadSong(m_downloadSongBtn);
            }
        }

        return true;
    }

    void onCopyLevelNow(CCObject*) {
        if (!m_fields->m_levelRef) return;
        m_fields->m_levelRef->m_password = 1;
        this->onClone(nullptr);
    }
};
