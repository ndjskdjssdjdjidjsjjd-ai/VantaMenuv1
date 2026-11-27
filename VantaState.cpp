#include "VantaState.hpp"

void VantaState::loadSavedMeta() {
    auto* mod = geode::Mod::get();
    lastOpenedCategory = mod->getSavedValue<std::string>("last_opened_category", "Player");
    customBgImagePath = mod->getSavedValue<std::string>("custom_bg_image_path", "");
    hasSavedVantaBtnPos = mod->getSavedValue<bool>("has_vanta_btn_pos", false);
    vantaBtnX = mod->getSavedValue<float>("vanta_btn_x", 0.f);
    vantaBtnY = mod->getSavedValue<float>("vanta_btn_y", 0.f);
}

void VantaState::setVantaBtnPos(float x, float y) {
    auto* mod = geode::Mod::get();
    hasSavedVantaBtnPos = true;
    vantaBtnX = x;
    vantaBtnY = y;
    mod->setSavedValue<bool>("has_vanta_btn_pos", true);
    mod->setSavedValue<float>("vanta_btn_x", x);
    mod->setSavedValue<float>("vanta_btn_y", y);
}

ccColor4F VantaState::getHitboxTrailColor() {
    int idx = static_cast<int>(std::round(getFloat("hitbox_trail_color")));
    switch (idx) {
        case 1: return {1.00f, 0.15f, 0.25f, 0.85f};
        case 2: return {0.15f, 1.00f, 0.45f, 0.85f};
        case 3: return {1.00f, 0.72f, 0.10f, 0.85f};
        case 4: return {0.72f, 0.28f, 1.00f, 0.85f};
        case 5: return {1.00f, 1.00f, 1.00f, 0.85f};
        default: return {0.15f, 0.88f, 1.00f, 0.85f};
    }
}

ccColor3B VantaState::getBuildingColorRGB() {
    int idx = static_cast<int>(std::round(getFloat("building_color")));
    switch (idx) {
        case 1: return {115, 205, 255};
        case 2: return {255, 95, 110};
        case 3: return {90, 240, 145};
        case 4: return {255, 205, 80};
        case 5: return {190, 115, 255};
        case 6: return {140, 150, 168};
        default: return {255, 255, 255};
    }
}

std::string VantaState::getActiveCustomTexturePath() {
    int idx = static_cast<int>(std::round(getFloat("custom_texture_slot")));
    if (idx >= 0 && idx < static_cast<int>(customTexturePaths.size())) {
        return customTexturePaths[idx];
    }
    return customTexturePaths.front();
}

bool VantaState::isCheating() {
    return getBool("noclip") || getBool("jump_hack") || getBool("no_spike") ||
           getBool("no_solid") || getBool("freeze_player") || getBool("instant_complete") ||
           (getBool("speedhack_en") && std::abs(getFloat("speed_val") - 1.0f) > 0.001f);
}
