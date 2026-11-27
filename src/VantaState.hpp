    // Inside VantaState class:
    float vantaBtnX = -1.f;
    float vantaBtnY = -1.f;
    bool hasSavedVantaBtnPos = false;

    void loadSavedMeta() {
        lastOpenedCategory = Mod::get()->getSavedValue<std::string>("last_opened_category", "Player");
        customBgImagePath = Mod::get()->getSavedValue<std::string>("custom_bg_image_path", "");
        hasSavedVantaBtnPos = Mod::get()->getSavedValue<bool>("has_vanta_btn_pos", false);
        vantaBtnX = Mod::get()->getSavedValue<float>("vanta_btn_x", 0.f);
        vantaBtnY = Mod::get()->getSavedValue<float>("vanta_btn_y", 0.f);
    }

    void setVantaBtnPos(float x, float y) {
        hasSavedVantaBtnPos = true;
        vantaBtnX = x;
        vantaBtnY = y;
        Mod::get()->setSavedValue<bool>("has_vanta_btn_pos", true);
        Mod::get()->setSavedValue<float>("vanta_btn_x", x);
        Mod::get()->setSavedValue<float>("vanta_btn_y", y);
    }
