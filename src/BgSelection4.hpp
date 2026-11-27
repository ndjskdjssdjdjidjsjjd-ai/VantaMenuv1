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
        float t,
        const std::string& imagePath,
        float opacity
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
};
