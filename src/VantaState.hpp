#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>

using namespace geode::prelude;

struct FeatureItem {
    std::string id;
    std::string name;
    std::string category;
    std::string desc;
    bool isSlider = false;
    float minVal = 0.0f;
    float maxVal = 1.0f;
    float defaultVal = 0.0f;
    std::vector<std::string> choices;
};

class VantaState {
public:
    static VantaState& get() {
        static VantaState instance;
        return instance;
    }

    std::vector<FeatureItem> features;
    std::unordered_map<std::string, bool> boolStates;
    std::unordered_map<std::string, float> floatStates;
    std::unordered_map<std::string, float> savedTabScrollY;

    std::string lastOpenedCategory = "Player";
    std::string customBgImagePath = "";
    std::vector<std::string> customTexturePaths = {
        "/storage/emulated/0/Download/vanta_texture.png",
        "/sdcard/Download/custom_icon.png",
        "GJ_square01.png"
    };

    float vantaBtnX = 0.f;
    float vantaBtnY = 0.f;
    bool hasSavedVantaBtnPos = false;

    int currentDeaths = 0;
    int totalClicks = 0;
    int sessionAttempts = 0;
    float noclipDeaths = 0.0f;
    float noclipFrames = 0.0f;
    int selectedStartPos = -1;

    void loadSavedMeta();
    void setVantaBtnPos(float x, float y);
    void initFeatures();

    bool isMenuHidden() {
        return geode::Mod::get()->getSettingValue<bool>("hide-menu");
    }

    bool getBool(const std::string& id) {
        return boolStates[id];
    }

    void setBool(const std::string& id, bool val) {
        boolStates[id] = val;
        geode::Mod::get()->setSavedValue<bool>(id, val);
    }

    float getFloat(const std::string& id) {
        return floatStates[id];
    }

    void setFloat(const std::string& id, float val) {
        floatStates[id] = val;
        geode::Mod::get()->setSavedValue<float>(id, val);
    }

    void setLastOpenedCategory(const std::string& cat) {
        lastOpenedCategory = cat;
        geode::Mod::get()->setSavedValue<std::string>("last_opened_category", cat);
    }

    void setCustomBgImagePath(const std::string& path) {
        customBgImagePath = path;
        geode::Mod::get()->setSavedValue<std::string>("custom_bg_image_path", path);
    }

    ccColor4F getHitboxTrailColor();
    ccColor3B getBuildingColorRGB();
    std::string getActiveCustomTexturePath();
    bool isCheating();
};
