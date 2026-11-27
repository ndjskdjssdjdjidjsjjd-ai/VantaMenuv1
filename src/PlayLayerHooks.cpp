#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <deque>
#include <cmath>
#include <algorithm>
#include "VantaState.hpp"
#include "HudManager.hpp"

using namespace geode::prelude;

class $modify(VantaPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_hudLabel = nullptr;
        CCDrawNode* m_cheatDot = nullptr;
        CCDrawNode* m_hitboxNode = nullptr;
        std::deque<CCRect> m_hitboxHistory;
        int m_frameCounter = 0;
        float m_fpsTimer = 0.0f;
        int m_displayedFps = 0;
        float m_rainbowHue = 0.0f;
    };

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

        auto winSize = CCDirector::sharedDirector()->getWinSize();

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

        if (state.getBool("instant_complete")) {
            this->levelComplete();
        }

        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        auto& state = VantaState::get();

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
        m_fields->m_hitboxHistory.clear();
    }

    void levelComplete() {
        auto& state = VantaState::get();
        if (state.getBool("safe_mode") || (state.getBool("auto_safe_mode") && state.isCheating())) {
            this->resetLevel();
            FLAlertLayer::create("VantaMenu Safe Mode", "Level completion blocked to protect account progress.", "OK")->show();
            return;
        }
        PlayLayer::levelComplete();
    }

    void postUpdate(float dt) {
        auto& state = VantaState::get();

        if (state.getBool("speedhack_en")) {
            dt *= state.getFloat("speed_val");
        }

        PlayLayer::postUpdate(dt);

        m_fields->m_frameCounter++;
        m_fields->m_fpsTimer += CCDirector::sharedDirector()->getDeltaTime();
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

        m_fields->m_hitboxNode->clear();
        if ((state.getBool("show_hitboxes") || state.getBool("hitbox_trail")) && m_player1) {
            CCRect pRect = m_player1->getObjectRect();
            if (state.getBool("hitbox_trail")) {
                m_fields->m_hitboxHistory.push_back(pRect);
                size_t maxFrames = static_cast<size_t>(state.getFloat("hitbox_trail_len"));
                while (m_fields->m_hitboxHistory.size() > maxFrames) {
                    m_fields->m_hitboxHistory.pop_front();
                }

                for (const auto& rect : m_fields->m_hitboxHistory) {
                    CCPoint verts[4] = {
                        {rect.getMinX(), rect.getMinY()},
                        {rect.getMinX(), rect.getMaxY()},
                        {rect.getMaxX(), rect.getMaxY()},
                        {rect.getMaxX(), rect.getMinY()}
                    };
                    m_fields->m_hitboxNode->drawPolygon(verts, 4, {1.f, 0.f, 0.f, 0.05f}, 0.5f, {1.f, 0.2f, 0.2f, 0.45f});
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
                m_fields->m_hitboxNode->drawPolygon(verts, 4, {1.f, 0.f, 0.f, fillOp}, 1.0f, {1.f, 0.f, 0.f, 1.0f});
            }
        }

        updateHUD();
    }

    void updateHUD() {
        auto& state = VantaState::get();
        bool hasPlayer = (m_player1 != nullptr);
        CCPoint pos = hasPlayer ? m_player1->getPosition() : CCPoint{0.f, 0.f};
        float vel = hasPlayer ? m_player1->m_yVelocity : 0.f;

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
