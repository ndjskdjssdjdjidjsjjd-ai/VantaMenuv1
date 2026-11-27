#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <cmath>
#include "VantaState.hpp"

using namespace geode::prelude;

struct AmbientParticle {
    float x, y, depth;
    float vx, vy;
    float size;
    float phase;
};

class Bg3DRenderer {
public:
    static void renderBackground(
        CCDrawNode* node,
        CCSprite* customSprite,
        CCSize winSize,
        CCPoint camPos,
        float timeSec,
        std::vector<AmbientParticle>& particles
    ) {
        if (!node) return;
        node->clear();

        auto& state = VantaState::get();
        int theme = static_cast<int>(std::round(state.getFloat("bg_theme")));
        float speed = state.getFloat("bg_anim_speed");
        float t = timeSec * speed;

        if (customSprite) {
            customSprite->setVisible(false);
        }

        if (theme <= 0) return;

        if (particles.empty()) {
            for (int i = 0; i < 64; ++i) {
                float depth = 0.25f + (i % 5) * 0.20f;
                particles.push_back({
                    static_cast<float>((i * 137) % 1600) - 200.f,
                    static_cast<float>((i * 83) % 900) - 100.f,
                    depth,
                    ((i % 7) - 3) * 8.f,
                    ((i % 5) - 2) * 10.f,
                    (2.2f + (i % 4) * 1.3f) * depth,
                    static_cast<float>(i) * 0.7f
                });
            }
        }

        // Custom file image mode
        if (theme == 5) {
            if (!state.customBgImagePath.empty() && customSprite) {
                auto tex = CCTextureCache::sharedTextureCache()->addImage(
                    state.customBgImagePath.c_str(), false
                );
                if (tex) {
                    customSprite->setTexture(tex);
                    customSprite->setVisible(true);
                    float px = winSize.width * 0.5f - (camPos.x * 0.02f) + std::sin(t * 0.7f) * 6.f;
                    float py = winSize.height * 0.5f - (camPos.y * 0.015f) + std::cos(t * 0.5f) * 4.f;
                    customSprite->setPosition({px, py});

                    auto texSize = tex->getContentSize();
                    if (texSize.width > 0.f && texSize.height > 0.f) {
                        float scaleX = (winSize.width * 1.12f) / texSize.width;
                        float scaleY = (winSize.height * 1.12f) / texSize.height;
                        customSprite->setScale(std::max(scaleX, scaleY));
                    }
                }
            }
            return;
        }

        ccColor4F skyTop = {0.1f, 0.2f, 0.4f, 0.55f};
        ccColor4F skyMid = {0.2f, 0.4f, 0.6f, 0.55f};
        ccColor4F farRidge = {0.15f, 0.28f, 0.45f, 0.55f};
        ccColor4F nearRidge = {0.09f, 0.18f, 0.32f, 0.65f};
        ccColor4F sunGlow = {1.0f, 0.95f, 0.80f, 0.25f};
        ccColor4F moteCol = {1.0f, 1.0f, 1.0f, 0.85f};

        switch (theme) {
            case 1: // Realistic Winter (Frost sky, snow peaks, drifting snow & wind)
                skyTop = {0.07f, 0.15f, 0.28f, 0.68f};
                skyMid = {0.42f, 0.66f, 0.88f, 0.62f};
                farRidge = {0.68f, 0.83f, 0.96f, 0.65f};
                nearRidge = {0.82f, 0.91f, 0.98f, 0.72f};
                sunGlow = {0.80f, 0.92f, 1.00f, 0.22f};
                moteCol = {0.96f, 0.99f, 1.00f, 0.90f};
                break;

            case 2: // Realistic Spring (Fresh morning sky, lush rolling hills, soft pollen/petals)
                skyTop = {0.16f, 0.45f, 0.78f, 0.62f};
                skyMid = {0.62f, 0.88f, 0.68f, 0.58f};
                farRidge = {0.26f, 0.62f, 0.34f, 0.62f};
                nearRidge = {0.15f, 0.46f, 0.22f, 0.70f};
                sunGlow = {1.00f, 0.96f, 0.72f, 0.28f};
                moteCol = {0.88f, 1.00f, 0.62f, 0.82f};
                break;

            case 3: // Realistic Summer (Warm golden-azure sky, sunlit dunes/hills, warm sunbeams/motes)
                skyTop = {0.05f, 0.38f, 0.84f, 0.65f};
                skyMid = {0.98f, 0.76f, 0.32f, 0.58f};
                farRidge = {0.20f, 0.52f, 0.36f, 0.58f};
                nearRidge = {0.12f, 0.36f, 0.24f, 0.68f};
                sunGlow = {1.00f, 0.86f, 0.35f, 0.35f};
                moteCol = {1.00f, 0.92f, 0.52f, 0.80f};
                break;

            case 4: // Realistic Cherry Blossom (Soft twilight/dusk rose sky, sakura silhouettes, drifting petals)
                skyTop = {0.22f, 0.11f, 0.27f, 0.68f};
                skyMid = {0.94f, 0.64f, 0.78f, 0.62f};
                farRidge = {0.58f, 0.30f, 0.46f, 0.62f};
                nearRidge = {0.36f, 0.16f, 0.29f, 0.72f};
                sunGlow = {1.00f, 0.78f, 0.88f, 0.28f};
                moteCol = {1.00f, 0.80f, 0.89f, 0.92f};
                break;

            default:
                break;
        }

        // Layered Sky Bands
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

        // Soft Sun / Atmospheric Glow
        float sunX = winSize.width * 0.76f - std::fmod(camPos.x * 0.015f, winSize.width * 0.25f);
        float sunY = winSize.height * 0.72f + std::sin(t * 0.6f) * 8.f;
        node->drawDot({sunX, sunY}, 56.f + std::sin(t * 1.4f) * 4.f, sunGlow);
        node->drawDot({sunX, sunY}, 24.f, {sunGlow.r, sunGlow.g, sunGlow.b, sunGlow.a * 1.4f});

        // Distant Parallax Ridge
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

        // Near Parallax Ridge
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

        // Realistic Animated Particles (Snowflakes, Petals, Pollen/Sun Motes — No Birds)
        for (auto& p : particles) {
            float dtStep = (1.f / 60.f) * speed;
            if (theme == 1) { // Snowfall with realistic gust drift
                p.y -= (24.f + p.depth * 26.f) * dtStep;
                p.x += (std::sin(t * 1.6f + p.phase) * 20.f + 12.f) * dtStep;
            } else if (theme == 4) { // Cherry blossom petals swaying & falling
                p.y -= (16.f + p.depth * 18.f) * dtStep;
                p.x += (std::sin(t * 2.1f + p.phase) * 28.f + 10.f) * dtStep;
            } else if (theme == 2) { // Spring floating petals/pollen
                p.y += std::sin(t * 1.4f + p.phase) * 14.f * dtStep;
                p.x += (std::cos(t * 1.2f + p.phase) * 18.f + 8.f) * dtStep;
            } else { // Summer warm drifting light motes
                p.y += std::cos(t * 1.1f + p.phase) * 10.f * dtStep;
                p.x += (std::sin(t * 0.9f + p.phase) * 16.f + 6.f) * dtStep;
            }

            if (p.y < -25.f) p.y = winSize.height + 25.f;
            if (p.y > winSize.height + 25.f) p.y = -25.f;

            float sx = std::fmod(p.x - camPos.x * 0.14f * p.depth, winSize.width);
            if (sx < 0.f) sx += winSize.width;
            float sy = std::fmod(p.y - camPos.y * 0.08f * p.depth, winSize.height);
            if (sy < 0.f) sy += winSize.height;

            if (theme == 4 || theme == 2) {
                // Petal / leaf shape
                float angle = t * 2.5f + p.phase;
                float rx = p.size * (0.9f + 0.35f * std::cos(angle));
                float ry = p.size * 0.65f;
                CCPoint petal[4] = {
                    {sx - rx, sy},
                    {sx, sy + ry},
                    {sx + rx, sy},
                    {sx, sy - ry}
                };
                node->drawPolygon(petal, 4, moteCol, 0.f, moteCol);
            } else {
                node->drawDot({sx, sy}, p.size, moteCol);
            }
        }
    }
};
