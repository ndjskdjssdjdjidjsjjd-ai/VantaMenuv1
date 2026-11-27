#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <cmath>
#include "VantaState.hpp"

using namespace geode::prelude;

struct Particle3D {
    float x, y, z;
    float vx, vy;
    float size;
};

class Bg3DRenderer {
public:
    static void render3DBackground(
        CCDrawNode* node,
        CCSprite* customSprite,
        CCSize winSize,
        CCPoint camPos,
        float timeSec,
        std::vector<Particle3D>& particles
    ) {
        if (!node) return;
        node->clear();

        auto& state = VantaState::get();
        int theme = static_cast<int>(std::round(state.getFloat("bg_3d_theme")));
        float depthScale = state.getFloat("bg_3d_depth");

        if (customSprite) {
            customSprite->setVisible(false);
        }

        if (theme <= 0) return; // Default GD background

        if (particles.empty()) {
            for (int i = 0; i < 42; ++i) {
                float depth = 0.25f + (i % 6) * 0.22f;
                particles.push_back({
                    static_cast<float>((i * 97) % 1200) - 200.f,
                    static_cast<float>((i * 61) % 800) - 100.f,
                    depth,
                    ((i % 5) - 2) * 14.f,
                    ((i % 7) - 3) * 18.f,
                    (2.5f + (i % 4) * 1.4f) * depth
                });
            }
        }

        // 3D Perspective Sky & Horizon Gradient Bands
        ccColor4F topCol = {0.06f, 0.08f, 0.16f, 0.45f};
        ccColor4F botCol = {0.12f, 0.18f, 0.28f, 0.45f};
        ccColor4F gridCol = {0.35f, 0.55f, 0.85f, 0.22f};
        ccColor4F pCol = {0.85f, 0.95f, 1.0f, 0.75f};

        switch (theme) {
            case 1: // Winter (3D)
                topCol = {0.07f, 0.16f, 0.30f, 0.55f};
                botCol = {0.45f, 0.72f, 0.95f, 0.55f};
                gridCol = {0.75f, 0.92f, 1.0f, 0.28f};
                pCol = {0.95f, 0.98f, 1.0f, 0.85f};
                break;
            case 2: // Fire (3D)
                topCol = {0.18f, 0.04f, 0.03f, 0.60f};
                botCol = {0.75f, 0.18f, 0.03f, 0.55f};
                gridCol = {1.0f, 0.45f, 0.08f, 0.30f};
                pCol = {1.0f, 0.65f, 0.15f, 0.85f};
                break;
            case 3: // Spring (3D)
                topCol = {0.14f, 0.35f, 0.22f, 0.50f};
                botCol = {0.55f, 0.85f, 0.42f, 0.50f};
                gridCol = {0.70f, 0.95f, 0.55f, 0.24f};
                pCol = {0.85f, 1.0f, 0.60f, 0.80f};
                break;
            case 4: // Summer (3D)
                topCol = {0.06f, 0.38f, 0.78f, 0.50f};
                botCol = {0.98f, 0.74f, 0.25f, 0.50f};
                gridCol = {1.0f, 0.85f, 0.45f, 0.25f};
                pCol = {1.0f, 0.92f, 0.40f, 0.85f};
                break;
            case 5: // Cherry Blossom (3D)
                topCol = {0.26f, 0.12f, 0.24f, 0.55f};
                botCol = {0.96f, 0.62f, 0.78f, 0.55f};
                gridCol = {1.0f, 0.75f, 0.88f, 0.28f};
                pCol = {1.0f, 0.82f, 0.90f, 0.88f};
                break;
            case 6: // Galaxy (3D)
                topCol = {0.04f, 0.02f, 0.12f, 0.72f};
                botCol = {0.25f, 0.08f, 0.45f, 0.65f};
                gridCol = {0.65f, 0.35f, 1.0f, 0.28f};
                pCol = {0.85f, 0.75f, 1.0f, 0.90f};
                break;
            case 7: // Custom Image (3D)
                topCol = {0.05f, 0.05f, 0.10f, 0.35f};
                botCol = {0.10f, 0.10f, 0.20f, 0.35f};
                gridCol = {0.4f, 0.7f, 1.0f, 0.18f};
                pCol = {1.0f, 1.0f, 1.0f, 0.75f};
                if (customSprite) {
                    std::string texPath = state.getActiveCustomTexturePath();
                    auto tex = CCTextureCache::sharedTextureCache()->addImage(texPath.c_str(), false);
                    if (tex) {
                        customSprite->setTexture(tex);
                        customSprite->setVisible(true);
                        float shiftX = -camPos.x * 0.03f * depthScale + std::sin(timeSec * 0.8f) * 10.f;
                        float shiftY = -camPos.y * 0.02f * depthScale + std::cos(timeSec * 0.6f) * 6.f;
                        customSprite->setPosition({winSize.width * 0.5f + shiftX, winSize.height * 0.5f + shiftY});
                        customSprite->setScale(1.12f + 0.03f * std::sin(timeSec));
                    }
                }
                break;
            default:
                break;
        }

        CCPoint skyQuad[4] = {
            {0.f, winSize.height * 0.45f},
            {winSize.width, winSize.height * 0.45f},
            {winSize.width, winSize.height},
            {0.f, winSize.height}
        };
        node->drawPolygon(skyQuad, 4, topCol, 0.f, topCol);

        CCPoint groundQuad[4] = {
            {0.f, 0.f},
            {winSize.width, 0.f},
            {winSize.width, winSize.height * 0.45f},
            {0.f, winSize.height * 0.45f}
        };
        node->drawPolygon(groundQuad, 4, botCol, 0.f, botCol);

        // Perspective 3D Floor Grid Converging Toward Horizon
        float horizonY = winSize.height * 0.45f;
        float vanishX = winSize.width * 0.5f - (camPos.x * 0.02f * depthScale);
        float scrollOffset = std::fmod(camPos.x * 0.08f * depthScale + timeSec * 18.f, 60.f);

        for (int i = -8; i <= 8; ++i) {
            float baseX = winSize.width * 0.5f + i * 70.f - scrollOffset;
            node->drawSegment({baseX, 0.f}, {vanishX + i * 18.f, horizonY}, 0.8f, gridCol);
        }

        for (int row = 1; row <= 6; ++row) {
            float t = static_cast<float>(row) / 6.f;
            float y = horizonY * (1.f - t * t);
            node->drawSegment({0.f, y}, {winSize.width, y}, 0.8f, gridCol);
        }

        // Animated 3D Depth Particles (Snow, Embers, Petals, Stars, Pollen)
        for (auto& p : particles) {
            float parallax = p.z * depthScale;
            if (theme == 1) { // Snow falling + drifting
                p.y -= (28.f + p.z * 22.f) * (1.f / 60.f);
                p.x += std::sin(timeSec * 1.5f + p.z * 4.f) * 0.45f;
            } else if (theme == 2) { // Fire embers rising
                p.y += (36.f + p.z * 25.f) * (1.f / 60.f);
                p.x += std::cos(timeSec * 2.2f + p.z * 5.f) * 0.65f;
            } else if (theme == 5) { // Cherry blossom petals drifting
                p.y -= (18.f + p.z * 14.f) * (1.f / 60.f);
                p.x += std::sin(timeSec * 1.8f + p.y * 0.02f) * 0.85f;
            } else { // Galaxy stars / spring pollen
                p.x += std::cos(timeSec + p.z) * 0.25f;
                p.y += std::sin(timeSec * 0.8f + p.z) * 0.25f;
            }

            if (p.y < -20.f) p.y = winSize.height + 20.f;
            if (p.y > winSize.height + 20.f) p.y = -20.f;

            float screenX = std::fmod(p.x - camPos.x * 0.12f * parallax, winSize.width);
            if (screenX < 0.f) screenX += winSize.width;
            float screenY = std::fmod(p.y - camPos.y * 0.06f * parallax, winSize.height);
            if (screenY < 0.f) screenY += winSize.height;

            node->drawDot({screenX, screenY}, p.size * (0.7f + 0.3f * depthScale), pCol);
        }
    }
};
