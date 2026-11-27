#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <cmath>
#include "VantaState.hpp"
#include "BgSelection1.hpp"
#include "BgSelection2.hpp"
#include "BgSelection3.hpp"
#include "BgSelection4.hpp"

using namespace geode::prelude;

class Bg3DRenderer {
public:
    static void renderBackground(
        CCDrawNode* node,
        CCSprite* customSprite,
        CCSize winSize,
        CCPoint camPos,
        float timeSec,
        std::vector<SeasonParticle>& particles
    ) {
        if (!node) return;
        node->clear();

        auto& state = VantaState::get();
        int season = static_cast<int>(std::round(state.getFloat("bg_season_theme")));
        bool customBgEn = state.getBool("custom_bg_en");
        float speed = state.getFloat("bg_anim_speed");
        float t = timeSec * speed;

        if (customBgEn) {
            BgSelection4::renderCustomImage(
                customSprite,
                winSize,
                camPos,
                timeSec,
                state.customBgImagePath,
                state.getFloat("custom_bg_opacity"),
                state.getFloat("custom_bg_motion")
            );
        } else if (customSprite) {
            customSprite->setVisible(false);
        }

        if (season <= 0) return;

        if (particles.empty()) {
            for (int i = 0; i < 64; ++i) {
                float depth = 0.25f + (i % 5) * 0.20f;
                particles.push_back({
                    static_cast<float>((i * 137) % 1600) - 200.f,
                    static_cast<float>((i * 83) % 900) - 100.f,
                    depth,
                    (2.4f + (i % 4) * 1.4f) * depth,
                    static_cast<float>(i) * 0.7f
                });
            }
        }

        switch (season) {
            case 1:
                BgSelection1::renderWinter(node, winSize, camPos, t, speed, particles);
                break;
            case 2:
                BgSelection2::renderSpring(node, winSize, camPos, t, speed, particles);
                break;
            case 3:
                BgSelection2::renderSummer(node, winSize, camPos, t, speed, particles);
                break;
            case 4:
                BgSelection3::renderCherryBlossom(node, winSize, camPos, t, speed, particles);
                break;
            default:
                BgSelection1::renderDefault(node, winSize);
                break;
        }
    }
};
