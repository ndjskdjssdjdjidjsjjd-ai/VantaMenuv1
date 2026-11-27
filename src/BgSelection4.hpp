#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <cmath>
#include <algorithm>

using namespace geode::prelude;

class BgSelection4 {
public:
    static void renderCustomImage(
        CCSprite* customSprite,
        CCSize winSize,
        CCPoint camPos,
        float timeSec,
        const std::string& imagePath,
        float opacity,
        float motionIntensity
    ) {
        if (!customSprite) return;

        if (imagePath.empty()) {
            customSprite->setVisible(false);
            return;
        }

        auto tex = CCTextureCache::sharedTextureCache()->addImage(imagePath.c_str(), false);
        if (!tex) {
            customSprite->setVisible(false);
            return;
        }

        customSprite->setTexture(tex);
        customSprite->setVisible(true);
        customSprite->setOpacity(
            static_cast<GLubyte>(std::clamp(opacity, 0.05f, 1.0f) * 255.f)
        );

        float amp = std::clamp(motionIntensity, 0.1f, 3.0f);
        // Automatic photo animation: smooth breathing scale + gentle Ken Burns pan/drift
        float autoZoom = 1.14f + 0.045f * amp * std::sin(timeSec * 0.65f);
        float driftX = std::sin(timeSec * 0.45f) * 18.f * amp - (camPos.x * 0.015f);
        float driftY = std::cos(timeSec * 0.38f) * 12.f * amp - (camPos.y * 0.01f);

        customSprite->setPosition({winSize.width * 0.5f + driftX, winSize.height * 0.5f + driftY});

        auto texSize = tex->getContentSize();
        if (texSize.width > 0.f && texSize.height > 0.f) {
            float coverScale = std::max(winSize.width / texSize.width, winSize.height / texSize.height);
            customSprite->setScale(coverScale * autoZoom);
        }
    }
};
