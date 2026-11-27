#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <algorithm>
#include "VantaState.hpp"

using namespace geode::prelude;

class HudManager {
public:
    static std::string buildHudString(
        int fps,
        int totalClicks,
        float noclipDeaths,
        float noclipFrames,
        int sessionAttempts,
        CCPoint playerPos,
        float yVelocity,
        bool hasPlayer
    ) {
        auto& state = VantaState::get();
        std::string hudText;

        if (state.getBool("hud_fps")) {
            hudText += fmt::format("FPS: {}\n", fps);
        }
        if (state.getBool("hud_tps")) {
            float tps = state.getBool("tps_bypass_en") ? state.getFloat("tps_val") : 240.0f;
            hudText += fmt::format("TPS: {:.0f}\n", tps);
        }
        if (state.getBool("hud_cps")) {
            hudText += fmt::format("Clicks: {}\n", totalClicks);
        }
        if (state.getBool("hud_noclip_acc") && state.getBool("noclip")) {
            float acc = noclipFrames > 0.f
                ? std::max(0.f, 100.f * (1.f - (noclipDeaths / noclipFrames)))
                : 100.f;
            hudText += fmt::format("Accuracy: {:.2f}%\n", acc);
        }
        if (state.getBool("hud_noclip_deaths") && state.getBool("noclip")) {
            hudText += fmt::format("Noclip Deaths: {:.0f}\n", noclipDeaths);
        }
        if (state.getBool("hud_attempts")) {
            hudText += fmt::format("Session Att: {}\n", sessionAttempts);
        }
        if (state.getBool("hud_pos") && hasPlayer) {
            hudText += fmt::format("Pos: {:.1f}, {:.1f}\n", playerPos.x, playerPos.y);
        }
        if (state.getBool("hud_velocity") && hasPlayer) {
            hudText += fmt::format("Y-Vel: {:.2f}\n", yVelocity);
        }

        return hudText;
    }
};
