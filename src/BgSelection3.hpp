#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <cmath>
#include "BgSelection1.hpp"

using namespace geode::prelude;

class BgSelection3 {
public:
    static void renderCherryBlossom(
        CCDrawNode* node,
        CCSize winSize,
        CCPoint camPos,
        float t,
        float speed,
        std::vector<SeasonParticle>& particles
    ) {
        if (!node) return;

        ccColor4F skyTop = {0.20f, 0.10f, 0.26f, 0.72f};
        ccColor4F skyMid = {0.95f, 0.62f, 0.78f, 0.66f};
        ccColor4F farRidge = {0.58f, 0.28f, 0.45f, 0.66f};
        ccColor4F nearRidge = {0.35f, 0.15f, 0.28f, 0.76f};
        ccColor4F blossomGlow = {1.00f, 0.78f, 0.88f, 0.30f};
        ccColor4F petalCol = {1.00f, 0.80f, 0.89f, 0.94f};

        CCPoint upperSky[4] = {
            {0.f, winSize.height * 0.52f},
            {winSize.width, winSize.height * 0.52f},
            {winSize.width, winSize.height},
            {0.f, winSize.height}
        };
        node->drawPolygon(upperSky, 4, skyTop, 0.f, skyTop);

        CCPoint lowerSky[4] = {
            {0.f, 0.f},
            {winSize.width, 0.f},
            {winSize.width, winSize.height * 0.55f},
            {0.f, winSize.height * 0.55f}
        };
        node->drawPolygon(lowerSky, 4, skyMid, 0.f, skyMid);

        float sunX = winSize.width * 0.76f - std::fmod(camPos.x * 0.015f, winSize.width * 0.25f);
        float sunY = winSize.height * 0.72f + std::sin(t * 0.6f) * 8.f;
        node->drawDot({sunX, sunY}, 58.f + std::sin(t * 1.4f) * 4.f, blossomGlow);

        float farShift = -std::fmod(camPos.x * 0.05f + t * 8.f, 220.f);
        for (int s = -1; s < 7; ++s) {
            float x0 = farShift + s * 220.f;
            float baseH = winSize.height * 0.28f;
            float peakH = winSize.height * 0.44f + std::sin(s * 1.7f + t * 0.35f) * 14.f;
            CCPoint ridge[4] = {
                {x0, 0.f},
                {x0 + 110.f, peakH},
                {x0 + 220.f, baseH},
                {x0 + 220.f, 0.f}
            };
            node->drawPolygon(ridge, 4, farRidge, 0.f, farRidge);
        }

        float nearShift = -std::fmod(camPos.x * 0.11f + t * 16.f, 180.f);
        for (int s = -1; s < 8; ++s) {
            float x0 = nearShift + s * 180.f;
            float baseH = winSize.height * 0.14f;
            float peakH = winSize.height * 0.27f + std::cos(s * 2.1f - t * 0.45f) * 10.f;
            CCPoint ridge[4] = {
                {x0, 0.f},
                {x0 + 90.f, peakH},
                {x0 + 180.f, baseH},
                {x0 + 180.f, 0.f}
            };
            node->drawPolygon(ridge, 4, nearRidge, 0.f, nearRidge);
        }

        float dtStep = (1.f / 60.f) * speed;
        for (auto& p : particles) {
            p.y -= (18.f + p.depth * 20.f) * dtStep;
            p.x += (std::sin(t * 2.2f + p.phase) * 30.f + 12.f) * dtStep;

            if (p.y < -25.f) p.y = winSize.height + 25.f;
            if (p.y > winSize.height + 25.f) p.y = -25.f;

            float sx = std::fmod(p.x - camPos.x * 0.14f * p.depth, winSize.width);
            if (sx < 0.f) sx += winSize.width;
            float sy = std::fmod(p.y - camPos.y * 0.08f * p.depth, winSize.height);
            if (sy < 0.f) sy += winSize.height;

            float angle = t * 2.6f + p.phase;
            float rx = p.size * (0.95f + 0.35f * std::cos(angle));
            float ry = p.size * 0.65f;
            CCPoint petal[4] = {
                {sx - rx, sy},
                {sx, sy + ry},
                {sx + rx, sy},
                {sx, sy - ry}
            };
            node->drawPolygon(petal, 4, petalCol, 0.f, petalCol);
        }
    }
};
