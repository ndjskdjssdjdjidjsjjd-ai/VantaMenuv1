#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "VantaState.hpp"

using namespace geode::prelude;

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

        if (level && state.getBool("auto_song_download") && level->m_songID > 0) {
            MusicDownloadManager::sharedState()->downloadSong(level->m_songID);
        }

        return true;
    }

    void onCopyLevelNow(CCObject*) {
        if (!m_fields->m_levelRef) return;
        m_fields->m_levelRef->m_password = 1;
        this->onClone(nullptr);
    }
};
