#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <cmath>
#include "BgSelection1.hpp"

using namespace geode::prelude;

class BgSelection2 {
public:
    static void renderSpring(
        CCDrawNode* node,
        CCSize winSize,
        CCPoint camPos,
        float t,
        float speed,
        std::vector<SeasonParticle>& particles
    ) {
        if (!node) return;

        ccColor4F skyTop = {0.16f, 0.45f, 0.78f, 0.62f};
        ccColor4F skyMid = {0.62f, 0.88f, 0.68f, 0.58f};
        ccColor4F farRidge = {0.26f, 0.62f, 0.34f, 0.62f};
        ccColor4F nearRidge = {0.15f, 0.46f, 0.22f, 0.70f};
        ccColor4F sunGlow = {1.00f, 0.96f, 0.72f, 0.28f};
        ccColor4F leafCol = {0.88f, 1.00f, 0.62f, 0.82f};

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
        node->drawDot({sunX, sunY}, 56.f + std::sin(t * 1.4f) * 4.f, sunGlow);

        float farShift = -std::fmod(camPos.x * 0.05f + t * 6.f, 220.f);
        for (int s = -1; s < 7; ++s) {
            float x0 = farShift + s * 220.f;
            float baseH = winSize.height * 0.28f;
            float peakH = winSize.height * 0.44f + std::sin(s * 1.7f + t * 0.3f) * 14.f;
            CCPoint ridge[4] = {
                {x0, 0.f},
                {x0 + 110.f, peakH},
                {x0 + 220.f, baseH},
                {x0 + 220.f, 0.f}
            };
            node->drawPolygon(ridge, 4, farRidge, 0.f, farRidge);
        }

        float nearShift = -std::fmod(camPos.x * 0.11f + t * 14.f, 180.f);
        for (int s = -1; s < 8; ++s) {
            float x0 = nearShift + s * 180.f;
            float baseH = winSize.height * 0.14f;
            float peakH = winSize.height * 0.27f + std::cos(s * 2.1f - t * 0.4f) * 10.f;
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
            p.y += std::sin(t * 1.4f + p.phase) * 14.f * dtStep;
            p.x += (std::cos(t * 1.2f + p.phase) * 18.f + 8.f) * dtStep;

            if (p.y < -25.f) p.y = winSize.height + 25.f;
            if (p.y > winSize.height + 25.f) p.y = -25.f;

            float sx = std::fmod(p.x - camPos.x * 0.14f * p.depth, winSize.width);
            if (sx < 0.f) sx += winSize.width;
            float sy = std::fmod(p.y - camPos.y * 0.08f * p.depth, winSize.height);
            if (sy < 0.f) sy += winSize.height;

            float angle = t * 2.5f + p.phase;
            float rx = p.size * (0.9f + 0.35f * std::cos(angle));
            float ry = p.size * 0.65f;
            CCPoint petal[4] = {
                {sx - rx, sy},
                {sx, sy + ry},
                {sx + rx, sy},
                {sx, sy - ry}
            };
            node->drawPolygon(petal, 4, leafCol, 0.f, leafCol);
        }
    }

    static void renderSummer(
        CCDrawNode* node,
        CCSize winSize,
        CCPoint camPos,
        float t,
        float speed,
        std::vector<SeasonParticle>& particles
    ) {
        if (!node) return;

        ccColor4F skyTop = {0.05f, 0.38f, 0.84f, 0.65f};
        ccColor4F skyMid = {0.98f, 0.76f, 0.32f, 0.58f};
        ccColor4F farRidge = {0.20f, 0.52f, 0.36f, 0.58f};
        ccColor4F nearRidge = {0.12f, 0.36f, 0.24f, 0.68f};
        ccColor4F sunGlow = {1.00f, 0.86f, 0.35f, 0.35f};
        ccColor4F moteCol = {1.00f, 0.92f, 0.52f, 0.80f};

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
        node->drawDot({sunX, sunY}, 62.f + std::sin(t * 1.4f) * 5.f, sunGlow);

        float farShift = -std::fmod(camPos.x * 0.05f + t * 6.f, 220.f);
        for (int s = -1; s < 7; ++s) {
            float x0 = farShift + s * 220.f;
            float baseH = winSize.height * 0.28f;
            float peakH = winSize.height * 0.44f + std::sin(s * 1.7f + t * 0.3f) * 14.f;
            CCPoint ridge[4] = {
                {x0, 0.f},
                {x0 + 110.f, peakH},
                {x0 + 220.f, baseH},
                {x0 + 220.f, 0.f}
            };
            node->drawPolygon(ridge, 4, farRidge, 0.f, farRidge);
        }

        float nearShift = -std::fmod(camPos.x * 0.11f + t * 14.f, 180.f);
        for (int s = -1; s < 8; ++s) {
            float x0 = nearShift + s * 180.f;
            float baseH = winSize.height * 0.14f;
            float peakH = winSize.height * 0.27f + std::cos(s * 2.1f - t * 0.4f) * 10.f;
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
            p.y += std::cos(t * 1.1f + p.phase) * 10.f * dtStep;
            p.x += (std::sin(t * 0.9f + p.phase) * 16.f + 6.f) * dtStep;

            if (p.y < -25.f) p.y = winSize.height + 25.f;
            if (p.y > winSize.height + 25.f) p.y = -25.f;

            float sx = std::fmod(p.x - camPos.x * 0.14f * p.depth, winSize.width);
            if (sx < 0.f) sx += winSize.width;
            float sy = std::fmod(p.y - camPos.y * 0.08f * p.depth, winSize.height);
            if (sy < 0.f) sy += winSize.height;

            node->drawDot({sx, sy}, p.size, moteCol);
        }
    }
};
