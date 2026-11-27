#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include "VantaState.hpp"

using namespace geode::prelude;

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
