#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <deque>
#include <vector>
#include <cmath>
#include <algorithm>
#include "VantaState.hpp"
#include "HudManager.hpp"
#include "BgSelection1.hpp"
#include "Bg3DRenderer.hpp"

using namespace geode::prelude;

class $modify(VantaPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_hudLabel = nullptr;
        CCDrawNode* m_cheatDot = nullptr;
        CCDrawNode* m_hitboxNode = nullptr;
        CCDrawNode* m_bg3DNode = nullptr;
        CCSprite* m_customBgSprite = nullptr;
        CCMenu* m_startPosMenu = nullptr;
        CCLabelBMFont* m_startPosLabel = nullptr;
        std::vector<StartPosObject*> m_startPositions;
        std::deque<CCRect> m_hitboxHistory;
        std::deque<CCPoint> m_velHistory;
        std::vector<SeasonParticle> m_bgParticles;
        int m_frameCounter = 0;
        float m_fpsTimer = 0.0f;
        int m_displayedFps = 0;
        float m_rainbowHue = 0.0f;
        float m_animTime = 0.0f;
        bool m_customTexApplied = false;
        bool m_playerDiedThisAttempt = false;
    };

    void collectStartPositions() {
        m_fields->m_startPositions.clear();
        if (!m_objects) return;
        for (int i = 0; i < m_objects->count(); ++i) {
            auto obj = typeinfo_cast<StartPosObject*>(m_objects->objectAtIndex(i));
            if (obj) {
                m_fields->m_startPositions.push_back(obj);
            }
        }
        std::sort(m_fields->m_startPositions.begin(), m_fields->m_startPositions.end(),
            [](StartPosObject* a, StartPosObject* b) {
                return a->getPositionX() < b->getPositionX();
            }
        );
    }

    ccColor3B blendColor(ccColor3B baseCol, ccColor3B targetCol, float mix) {
        float t = std::clamp(mix, 0.f, 1.f);
        return {
            static_cast<GLubyte>(baseCol.r * (1.f - t) + targetCol.r * t),
            static_cast<GLubyte>(baseCol.g * (1.f - t) + targetCol.g * t),
            static_cast<GLubyte>(baseCol.b * (1.f - t) + targetCol.b * t)
        };
    }

    void applyBuildingCustomization() {
        auto& state = VantaState::get();
        float bOp = state.getFloat("building_opacity");
        bool simDeco = state.getBool("hide_deco_buildings");
        bool noGlow = state.getBool("no_glow");
        int colIdx = static_cast<int>(std::round(state.getFloat("building_color")));
        float colorMix = state.getFloat("building_color_mix");
        ccColor3B customCol = state.getBuildingColorRGB();

        if (!m_objects) return;
        GLubyte alphaByte = static_cast<GLubyte>(std::clamp(bOp, 0.05f, 1.0f) * 255.f);

        for (int i = 0; i < m_objects->count(); ++i) {
            auto gameObj = typeinfo_cast<GameObject*>(m_objects->objectAtIndex(i));
            if (!gameObj) continue;

            bool isSolid = (gameObj->m_objectType == GameObjectType::Solid);
            if (!isSolid && simDeco) {
                gameObj->setOpacity(static_cast<GLubyte>(alphaByte * 0.35f));
            } else {
                gameObj->setOpacity(alphaByte);
            }

            if (colIdx > 0 && colorMix > 0.01f) {
                ccColor3B blended = blendColor({255, 255, 255}, customCol, colorMix);
                gameObj->setColor(blended);
            }

            if (noGlow) {
                gameObj->m_hasNoGlow = true;
            }
        }
    }

    bool init(GJGameLevel* level, bool dontCreateObjects) {
        auto& state = VantaState::get();
        if (state.getBool("force_ldm") && level) {
            level->m_lowDetailModeToggled = true;
        }

        if (!PlayLayer::init(level, dontCreateObjects)) return false;

        state.sessionAttempts = 0;
        state.totalClicks = 0;
        state.noclipDeaths = 0.0f;
        state.noclipFrames = 0.0f;
        state.selectedStartPos = -1;
        m_fields->m_playerDiedThisAttempt = false;

        collectStartPositions();
        applyBuildingCustomization();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        m_fields->m_bg3DNode = CCDrawNode::create();
        m_fields->m_bg3DNode->setZOrder(-10);
        this->addChild(m_fields->m_bg3DNode);

        m_fields->m_customBgSprite = CCSprite::create();
        m_fields->m_customBgSprite->setVisible(false);
        m_fields->m_customBgSprite->setZOrder(-11);
        this->addChild(m_fields->m_customBgSprite);

        m_fields->m_hudLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_fields->m_hudLabel->setAnchorPoint({0.f, 1.f});
        m_fields->m_hudLabel->setPosition({12.f, winSize.height - 10.f});
        m_fields->m_hudLabel->setZOrder(500);
        this->addChild(m_fields->m_hudLabel);

        m_fields->m_cheatDot = CCDrawNode::create();
        m_fields->m_cheatDot->setPosition({winSize.width - 16.f, winSize.height - 16.f});
        m_fields->m_cheatDot->setZOrder(501);
        this->addChild(m_fields->m_cheatDot);

        m_fields->m_hitboxNode = CCDrawNode::create();
        m_fields->m_hitboxNode->setZOrder(450);
        if (m_objectLayer) {
            m_objectLayer->addChild(m_fields->m_hitboxNode);
        }

        m_fields->m_startPosMenu = CCMenu::create();
        m_fields->m_startPosMenu->setPosition({winSize.width * 0.5f, 34.f});
        m_fields->m_startPosMenu->setZOrder(510);
        m_fields->m_startPosMenu->setVisible(state.getBool("startpos_switcher"));
        this->addChild(m_fields->m_startPosMenu);

        auto prevSpr = ButtonSprite::create("< SP", 42, true, "bigFont.fnt", "GJ_button_04.png", 22.f, 0.45f);
        auto prevBtn = CCMenuItemSpriteExtra::create(prevSpr, this, menu_selector(VantaPlayLayer::onPrevStartPos));
        prevBtn->setPosition({-65.f, 0.f});
        m_fields->m_startPosMenu->addChild(prevBtn);

        m_fields->m_startPosLabel = CCLabelBMFont::create("SP: Start", "goldFont.fnt");
        m_fields->m_startPosLabel->setScale(0.45f);
        m_fields->m_startPosLabel->setPosition({0.f, 0.f});
        m_fields->m_startPosMenu->addChild(m_fields->m_startPosLabel);

        auto nextSpr = ButtonSprite::create("SP >", 42, true, "bigFont.fnt", "GJ_button_04.png", 22.f, 0.45f);
        auto nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(VantaPlayLayer::onNextStartPos));
        nextBtn->setPosition({65.f, 0.f});
        m_fields->m_startPosMenu->addChild(nextBtn);

        return true;
    }

    void updateStartPosLabel() {
        int total = static_cast<int>(m_fields->m_startPositions.size());
        int sel = VantaState::get().selectedStartPos;
        if (m_fields->m_startPosLabel) {
            if (sel < 0 || total == 0) {
                m_fields->m_startPosLabel->setString(fmt::format("SP: 0/{}", total).c_str());
            } else {
                m_fields->m_startPosLabel->setString(fmt::format("SP: {}/{}", sel + 1, total).c_str());
            }
        }
    }

    void jumpToSelectedStartPos() {
        auto& state = VantaState::get();
        int total = static_cast<int>(m_fields->m_startPositions.size());
        if (total > 0 && state.selectedStartPos >= 0 && state.selectedStartPos < total && m_player1) {
            auto sp = m_fields->m_startPositions[state.selectedStartPos];
            m_player1->setPosition(sp->getPosition());
        }
        updateStartPosLabel();
        this->resetLevel();
    }

    void onPrevStartPos(CCObject*) {
        int total = static_cast<int>(m_fields->m_startPositions.size());
        if (total <= 0) return;
        auto& state = VantaState::get();
        state.selectedStartPos--;
        if (state.selectedStartPos < -1) {
            state.selectedStartPos = total - 1;
        }
        jumpToSelectedStartPos();
    }

    void onNextStartPos(CCObject*) {
        int total = static_cast<int>(m_fields->m_startPositions.size());
        if (total <= 0) return;
        auto& state = VantaState::get();
        state.selectedStartPos++;
        if (state.selectedStartPos >= total) {
            state.selectedStartPos = -1;
        }
        jumpToSelectedStartPos();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        auto& state = VantaState::get();
        m_fields->m_playerDiedThisAttempt = true;

        if (state.getBool("noclip")) {
            state.noclipDeaths += 1.0f;
            return;
        }

        if (state.getBool("no_spike") && object && object->m_objectType == GameObjectType::Hazard) {
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        auto& state = VantaState::get();
        state.sessionAttempts++;
        state.noclipDeaths = 0.0f;
        state.noclipFrames = 0.0f;
        m_fields->m_playerDiedThisAttempt = false;
        m_fields->m_hitboxHistory.clear();
        m_fields->m_velHistory.clear();

        int total = static_cast<int>(m_fields->m_startPositions.size());
        if (state.getBool("startpos_switcher") && total > 0 && state.selectedStartPos >= 0 && state.selectedStartPos < total && m_player1) {
            auto sp = m_fields->m_startPositions[state.selectedStartPos];
            m_player1->setPosition(sp->getPosition());
        }
    }

    void postUpdate(float dt) {
        auto& state = VantaState::get();
        float rawDt = CCDirector::sharedDirector()->getDeltaTime();
        m_fields->m_animTime += rawDt;

        if (state.getBool("speedhack_en")) {
            dt *= state.getFloat("speed_val");
        }

        PlayLayer::postUpdate(dt);

        if (m_fields->m_startPosMenu) {
            m_fields->m_startPosMenu->setVisible(state.getBool("startpos_switcher"));
        }

        if (state.getBool("custom_texture_en") && m_player1 && !m_fields->m_customTexApplied) {
            std::string texPath = state.getActiveCustomTexturePath();
            auto tex = CCTextureCache::sharedTextureCache()->addImage(texPath.c_str(), false);
            if (tex) {
                m_player1->setTexture(tex);
                m_fields->m_customTexApplied = true;
            }
        } else if (!state.getBool("custom_texture_en")) {
            m_fields->m_customTexApplied = false;
        }

        m_fields->m_frameCounter++;
        m_fields->m_fpsTimer += rawDt;
        if (m_fields->m_fpsTimer >= 0.5f) {
            m_fields->m_displayedFps = static_cast<int>(m_fields->m_frameCounter / m_fields->m_fpsTimer);
            m_fields->m_frameCounter = 0;
            m_fields->m_fpsTimer = 0.0f;
        }

        if (state.getBool("noclip")) {
            state.noclipFrames += 1.0f;
        }

        if (state.getBool("rainbow_icon") && m_player1) {
            m_fields->m_rainbowHue += dt * 120.0f * state.getFloat("rainbow_speed");
            if (m_fields->m_rainbowHue > 360.0f) m_fields->m_rainbowHue -= 360.0f;
            ccColor3B col = {
                static_cast<GLubyte>(127 + 127 * sin(m_fields->m_rainbowHue * 0.01745f)),
                static_cast<GLubyte>(127 + 127 * sin((m_fields->m_rainbowHue + 120.f) * 0.01745f)),
                static_cast<GLubyte>(127 + 127 * sin((m_fields->m_rainbowHue + 240.f) * 0.01745f))
            };
            m_player1->setColor(col);
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        CCPoint camPos = m_player1 ? m_player1->getPosition() : CCPoint{0.f, 0.f};
        Bg3DRenderer::renderBackground(
            m_fields->m_bg3DNode,
            m_fields->m_customBgSprite,
            winSize,
            camPos,
            m_fields->m_animTime,
            m_fields->m_bgParticles
        );

        m_fields->m_hitboxNode->clear();
        if ((state.getBool("show_hitboxes") || state.getBool("hitbox_trail")) && m_player1) {
            CCRect pRect = m_player1->getObjectRect();
            CCPoint pCenter = {pRect.getMidX(), pRect.getMidY()};

            bool trailAllowed = state.getBool("hitbox_trail") &&
                (!state.getBool("hitbox_trail_death_only") || m_fields->m_playerDiedThisAttempt);

            if (trailAllowed) {
                m_fields->m_hitboxHistory.push_back(pRect);
                m_fields->m_velHistory.push_back(pCenter);
                size_t maxFrames = static_cast<size_t>(state.getFloat("hitbox_trail_len"));
                while (m_fields->m_hitboxHistory.size() > maxFrames) {
                    m_fields->m_hitboxHistory.pop_front();
                }
                while (m_fields->m_velHistory.size() > maxFrames) {
                    m_fields->m_velHistory.pop_front();
                }

                ccColor4F trailCol = state.getHitboxTrailColor();
                size_t totalHist = m_fields->m_hitboxHistory.size();
                for (size_t idx = 0; idx < totalHist; ++idx) {
                    const auto& rect = m_fields->m_hitboxHistory[idx];
                    float ageRatio = static_cast<float>(idx + 1) / static_cast<float>(std::max<size_t>(1, totalHist));
                    float alpha = (0.08f + 0.36f * ageRatio) * trailCol.a;

                    if (idx % 6 == 0 || idx + 1 == totalHist) {
                        CCPoint verts[4] = {
                            {rect.getMinX(), rect.getMinY()},
                            {rect.getMinX(), rect.getMaxY()},
                            {rect.getMaxX(), rect.getMaxY()},
                            {rect.getMaxX(), rect.getMinY()}
                        };
                        m_fields->m_hitboxNode->drawPolygon(
                            verts, 4,
                            {trailCol.r, trailCol.g, trailCol.b, alpha * 0.28f},
                            0.7f,
                            {trailCol.r, trailCol.g, trailCol.b, alpha}
                        );
                    } else {
                        m_fields->m_hitboxNode->drawDot(
                            {rect.getMidX(), rect.getMidY()},
                            1.8f,
                            {trailCol.r, trailCol.g, trailCol.b, alpha}
                        );
                    }
                }

                if (m_fields->m_velHistory.size() >= 2) {
                    for (size_t i = 1; i < m_fields->m_velHistory.size(); ++i) {
                        m_fields->m_hitboxNode->drawSegment(
                            m_fields->m_velHistory[i - 1],
                            m_fields->m_velHistory[i],
                            0.95f,
                            {trailCol.r, trailCol.g, trailCol.b, 0.72f}
                        );
                    }
                }
            }

            if (state.getBool("show_hitboxes")) {
                CCPoint verts[4] = {
                    {pRect.getMinX(), pRect.getMinY()},
                    {pRect.getMinX(), pRect.getMaxY()},
                    {pRect.getMaxX(), pRect.getMaxY()},
                    {pRect.getMaxX(), pRect.getMinY()}
                };
                float fillOp = state.getFloat("hitbox_opacity");
                m_fields->m_hitboxNode->drawPolygon(
                    verts, 4,
                    {1.f, 0.05f, 0.05f, fillOp},
                    1.25f,
                    {1.f, 0.1f, 0.1f, 1.0f}
                );

                float velX = static_cast<float>(m_player1->m_playerSpeed) * 22.f;
                float velY = static_cast<float>(m_player1->m_yVelocity) * 4.5f;
                CCPoint velEnd = CCPoint{pCenter.x + velX, pCenter.y + velY};
                m_fields->m_hitboxNode->drawSegment(pCenter, velEnd, 1.4f, {0.1f, 1.f, 0.45f, 0.95f});
                m_fields->m_hitboxNode->drawDot(velEnd, 3.0f, {0.1f, 1.f, 0.45f, 0.95f});
            }
        }

        updateHUD();
    }

    void updateHUD() {
        auto& state = VantaState::get();
        bool hasPlayer = (m_player1 != nullptr);
        CCPoint pos = hasPlayer ? m_player1->getPosition() : CCPoint{0.f, 0.f};
        float vel = hasPlayer ? static_cast<float>(m_player1->m_yVelocity) : 0.f;

        std::string hudText = HudManager::buildHudString(
            m_fields->m_displayedFps,
            state.totalClicks,
            state.noclipDeaths,
            state.noclipFrames,
            state.sessionAttempts,
            pos,
            vel,
            hasPlayer
        );

        m_fields->m_hudLabel->setString(hudText.c_str());
        m_fields->m_hudLabel->setScale(state.getFloat("hud_scale") * 0.65f);
        m_fields->m_hudLabel->setOpacity(static_cast<GLubyte>(state.getFloat("hud_opacity") * 255));

        m_fields->m_cheatDot->clear();
        if (state.getBool("hud_cheat_dot")) {
            ccColor4F dotColor = state.isCheating()
                ? ccColor4F{1.f, 0.15f, 0.15f, 0.95f}
                : ccColor4F{0.15f, 1.f, 0.25f, 0.95f};
            m_fields->m_cheatDot->drawDot({0.f, 0.f}, 5.5f, dotColor);
        }
    }
};

class $modify(VantaPlayerObject, PlayerObject) {
    void pushButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump) {
            VantaState::get().totalClicks++;
            if (VantaState::get().getBool("jump_hack")) {
                this->m_isOnGround = true;
            }
        }
        PlayerObject::pushButton(btn);
    }

    void update(float dt) {
        auto& state = VantaState::get();
        float gMult = state.getFloat("gravity_mult");
        if (std::abs(gMult - 1.0f) > 0.01f) {
            this->m_gravityMod = gMult;
        }
        PlayerObject::update(dt);
    }
};
