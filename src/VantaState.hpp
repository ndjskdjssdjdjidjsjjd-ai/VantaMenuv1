#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>

using namespace geode::prelude;

struct FeatureItem {
    std::string id;
    std::string name;
    std::string category;
    std::string desc;
    bool isSlider = false;
    float minVal = 0.0f;
    float maxVal = 1.0f;
    float defaultVal = 0.0f;
};

class VantaState {
public:
    static VantaState& get() {
        static VantaState instance;
        return instance;
    }

    std::vector<FeatureItem> features;
    std::unordered_map<std::string, bool> boolStates;
    std::unordered_map<std::string, float> floatStates;

    int currentDeaths = 0;
    int totalClicks = 0;
    int sessionAttempts = 0;
    float noclipDeaths = 0.0f;
    float noclipFrames = 0.0f;

    void initFeatures() {
        if (!features.empty()) return;

        auto addToggle = [&](std::string id, std::string name, std::string cat, std::string desc, bool def = false) {
            features.push_back({id, name, cat, desc, false, 0.f, 1.f, def ? 1.f : 0.f});
            boolStates[id] = Mod::get()->getSavedValue<bool>(id, def);
        };

        auto addSlider = [&](std::string id, std::string name, std::string cat, std::string desc, float minV, float maxV, float def) {
            features.push_back({id, name, cat, desc, true, minV, maxV, def});
            floatStates[id] = Mod::get()->getSavedValue<float>(id, def);
        };

        // --- 1. PLAYER & PHYSICS (20 Features) ---
        addToggle("noclip", "Noclip", "Player", "Prevents player death on obstacle collision.");
        addToggle("safe_mode", "Safe Mode", "Player", "Blocks level progress and leaderboard submission when cheats are active.", true);
        addToggle("auto_safe_mode", "Auto Safe Mode", "Player", "Automatically enables Safe Mode if Noclip or Speedhack is toggled.", true);
        addToggle("jump_hack", "Jump Hack", "Player", "Allows infinite mid-air jumping in any gamemode.");
        addToggle("no_spike", "Ignore Spikes", "Player", "Disables hazard hitboxes while keeping solid block collisions.");
        addToggle("no_solid", "Ignore Solids", "Player", "Allows passing through solid blocks while keeping hazard collisions.");
        addToggle("force_platformer", "Force Platformer", "Player", "Forces classic levels into 2.2 platformer physics.");
        addToggle("no_mirror", "Disable Mirror Portal", "Player", "Prevents screen flipping when entering mirror portals.");
        addToggle("no_gravity_flip", "Lock Gravity", "Player", "Prevents gravity portals from inverting player gravity.");
        addToggle("no_size_change", "Lock Player Size", "Player", "Ignores mini and normal size portals.");
        addToggle("dual_same_input", "Sync Dual Input", "Player", "Mirrors primary player inputs directly to the secondary dual icon.");
        addToggle("instant_respawn", "Instant Respawn", "Player", "Removes the respawn delay timer after crashing.");
        addToggle("no_respawn_blink", "No Respawn Blink", "Player", "Removes the invulnerability flashing animation on spawn.");
        addToggle("no_death_effect", "No Death Explosion", "Player", "Disables particle explosion when the player dies.");
        addToggle("no_death_shake", "No Death Screen Shake", "Player", "Disables camera shake upon crashing.");
        addToggle("freeze_player", "Freeze Player", "Player", "Locks player X/Y coordinates in place.");
        addToggle("anti_kick", "Bypass Kick/Anti-Cheat", "Player", "Bypasses internal speed and completion time sanity checks.");
        addToggle("no_robot_cap", "Uncapped Robot Jump", "Player", "Removes maximum hold duration limit on Robot jumps.");
        addToggle("no_wave_pulse", "Static Wave Trail", "Player", "Prevents wave trail thickness from pulsing with audio.");
        addSlider("gravity_mult", "Gravity Multiplier", "Player", "Scales vertical gravity acceleration.", 0.1f, 3.0f, 1.0f);

        // --- 2. SPEED & ENGINE BYPASS (15 Features) ---
        addToggle("fps_bypass_en", "Enable FPS Bypass", "Engine", "Unlocks frame rate cap above native refresh rate.");
        addSlider("fps_val", "Target FPS", "Engine", "Sets target frames per second.", 30.0f, 360.0f, 240.0f);
        addToggle("tps_bypass_en", "Enable TPS Bypass", "Engine", "Decouples physics ticks per second from render FPS.");
        addSlider("tps_val", "Physics TPS", "Engine", "Sets physics simulation ticks per second (240 = standard).", 60.0f, 480.0f, 240.0f);
        addToggle("speedhack_en", "Enable Speedhack", "Engine", "Modifies global game time scale.");
        addSlider("speed_val", "Game Speed", "Engine", "Multiplier for game speed (0.1x to 5.0x).", 0.1f, 5.0f, 1.0f);
        addToggle("speed_audio", "Pitch Audio with Speed", "Engine", "Synchronizes music pitch/tempo with Speedhack.");
        addToggle("cbf_sim", "Input Sub-Frame Poll", "Engine", "Processes touch/click events immediately between render frames.");
        addToggle("no_delta_cap", "Uncap Frame Delta", "Engine", "Prevents physics slowdown during heavy frame drops.");
        addToggle("force_ldm", "Force Low Detail (LDM)", "Engine", "Automatically toggles LDM on all levels before loading.");
        addToggle("ultra_ldm", "Ultra LDM (Strip Deco)", "Engine", "Hides non-collidable decoration objects for smooth performance.");
        addToggle("no_particles", "Disable All Particles", "Engine", "Stops particle emitters from spawning.");
        addToggle("no_glow", "Disable Object Glow", "Engine", "Forces additive glow blending off on all level objects.");
        addToggle("no_shaders", "Disable Shaders", "Engine", "Bypasses post-processing shader layers to save GPU overhead.");
        addToggle("fast_alt_tab", "No Pause on Focus Loss", "Engine", "Keeps game running when switching apps or windows.");

        // --- 3. HITBOXES & VISUALS (20 Features) ---
        addToggle("show_hitboxes", "Show Hitboxes", "Visuals", "Renders real-time collision boxes over objects and player.");
        addToggle("hitbox_on_death", "Hitboxes on Death Only", "Visuals", "Reveals collision boxes only after crashing.");
        addToggle("hitbox_trail", "Hitbox Trail", "Visuals", "Leaves a persistent trail of player hitboxes.");
        addSlider("hitbox_trail_len", "Hitbox Trail Frames", "Visuals", "Number of frames stored in hitbox trail.", 10.0f, 1000.0f, 240.0f);
        addSlider("hitbox_opacity", "Hitbox Fill Opacity", "Visuals", "Opacity of interior hitbox shading.", 0.0f, 1.0f, 0.25f);
        addToggle("no_trail", "Hide Player Trail", "Visuals", "Disables standard motion streak behind icon.");
        addToggle("force_trail", "Always Show Trail", "Visuals", "Forces player motion streak on even when disabled by triggers.");
        addToggle("solid_wave", "Solid Wave Trail", "Visuals", "Renders wave trail with solid opacity instead of fading.");
        addToggle("rainbow_icon", "RGB Rainbow Icon", "Visuals", "Cycles primary and secondary icon colors smoothly.");
        addSlider("rainbow_speed", "RGB Cycle Speed", "Visuals", "Speed of rainbow color transition.", 0.1f, 5.0f, 1.0f);
        addToggle("no_bg_pulse", "Disable BG Audio Pulse", "Visuals", "Stops background elements from pulsing to music.");
        addToggle("no_orb_pulse", "Disable Orb/Portal Pulse", "Visuals", "Locks ring and portal scale against music beat.");
        addToggle("no_camera_zoom", "Disable Camera Zoom", "Visuals", "Locks camera zoom triggers to 1.0x.");
        addToggle("no_camera_rotate", "Disable Camera Rotate", "Visuals", "Locks camera rotation triggers to 0 degrees.");
        addToggle("no_camera_offset", "Disable Camera Offset", "Visuals", "Centers camera on player, ignoring static/offset triggers.");
        addToggle("hide_attempts", "Hide Attempt Text", "Visuals", "Hides the in-level 'Attempt X' label.");
        addToggle("no_progress_bar", "Hide Progress Bar", "Visuals", "Removes top level progress bar.");
        addToggle("decimal_percent", "Decimal Percentage", "Visuals", "Displays level percentage to two decimal places (e.g., 45.82%).");
        addToggle("show_layout", "Layout Mode", "Visuals", "Forces solid white/black visibility on invisible blocks.");
        addToggle("hide_player", "Hide Player Icon", "Visuals", "Makes player sprite invisible while keeping hitboxes.");

        // --- 4. HUD & TELEMETRY LABELS (15 Features) ---
        addToggle("hud_fps", "HUD: Live FPS", "HUD", "Displays real-time frames per second on screen.");
        addToggle("hud_cps", "HUD: Live CPS & Total", "HUD", "Displays current clicks per second and total level clicks.");
        addToggle("hud_noclip_acc", "HUD: Noclip Accuracy", "HUD", "Shows exact survival frame percentage while using Noclip.");
        addToggle("hud_noclip_deaths", "HUD: Noclip Deaths", "HUD", "Counts lethal collisions avoided during Noclip.");
        addToggle("hud_cheat_dot", "HUD: Cheat Indicator", "HUD", "Displays green (legit) or red (cheated) status circle.");
        addToggle("hud_attempts", "HUD: Session Attempts", "HUD", "Displays total attempts across current play session.");
        addToggle("hud_clock", "HUD: Real-Time Clock", "HUD", "Displays local device time in the corner.");
        addToggle("hud_best_run", "HUD: Best Run %", "HUD", "Shows furthest percentage reached in current session.");
        addToggle("hud_pos", "HUD: Player X/Y Pos", "HUD", "Displays exact world coordinates of the player.");
        addToggle("hud_velocity", "HUD: Y-Velocity", "HUD", "Displays vertical momentum value.");
        addToggle("hud_level_id", "HUD: Level ID & Name", "HUD", "Displays current level metadata on screen.");
        addToggle("hud_tps", "HUD: Active TPS", "HUD", "Displays active physics tick rate.");
        addSlider("hud_scale", "HUD Text Scale", "HUD", "Adjusts font size of all VantaMenu HUD labels.", 0.3f, 1.2f, 0.55f);
        addSlider("hud_opacity", "HUD Opacity", "HUD", "Adjusts transparency of HUD overlay labels.", 0.1f, 1.0f, 0.8f);
        addToggle("hud_top_left", "HUD Anchor: Top-Left", "HUD", "Aligns telemetry labels to top-left corner.");

        // --- 5. PRACTICE & STARTPOS (15 Features) ---
        addToggle("practice_music", "Practice Music Hack", "Practice", "Plays normal level soundtrack instead of practice loop.");
        addToggle("auto_checkpoint", "Fast Auto-Checkpoints", "Practice", "Places practice checkpoints more frequently.");
        addToggle("no_checkpoint_lag", "No Checkpoint Save Lag", "Practice", "Optimizes state serialization when placing checkpoints.");
        addToggle("startpos_switcher", "StartPos Switcher", "Practice", "Enables on-screen arrows to cycle between StartPositions.");
        addToggle("smart_startpos", "Smart StartPos State", "Practice", "Auto-detects correct speed, mode, and gravity for StartPos.");
        addToggle("show_checkpoints", "Highlight Checkpoints", "Practice", "Makes practice checkpoint gems brighter.");
        addToggle("no_auto_retry", "Disable Auto-Retry", "Practice", "Prevents automatic level restart on death.");
        addToggle("instant_complete", "Instant Complete Level", "Practice", "Triggers level end wall immediately (requires Safe Mode off to save).");
        addToggle("confirm_exit", "Confirm Level Exit", "Practice", "Shows confirmation popup before quitting a level.");
        addToggle("confirm_restart", "Confirm Level Restart", "Practice", "Shows confirmation popup before restarting a level.");
        addToggle("no_orb_ring_click", "Ignore Buffer Clicks", "Practice", "Prevents buffered orb activations on respawn.");
        addToggle("hitbox_multiplier", "Forgiving Hitboxes (0.8x)", "Practice", "Shrinks hazard hitboxes by 20% for easier practice.");
        addToggle("auto_practice", "Auto-Enter Practice", "Practice", "Starts levels directly in Practice Mode.");
        addToggle("save_checkpoints", "Persist Checkpoints", "Practice", "Keeps practice checkpoints across level restarts.");
        addToggle("checkpoint_fix", "Fix Checkpoint Physics", "Practice", "Restores exact Y-velocity and rotation on checkpoint load.");

        // --- 6. BYPASSES & UTILITIES (15 Features) ---
        addToggle("icon_hack", "Unlock All Icons", "Bypass", "Unlocks all cubes, ships, balls, UFOs, waves, robots, spiders, swings.");
        addToggle("color_hack", "Unlock All Colors", "Bypass", "Unlocks all primary, secondary, and glow colors.");
        addToggle("copy_hack", "Level Copy Bypass", "Bypass", "Allows copying any online level without a password.");
        addToggle("no_copy_mark", "Remove (C) Mark", "Bypass", "Removes the copied level badge on published levels.");
        addToggle("verify_hack", "Bypass Verify Check", "Bypass", "Allows uploading levels without completing verification.");
        addToggle("object_limit", "Bypass Object Limit", "Bypass", "Removes editor object count limits.");
        addToggle("custom_obj_limit", "Bypass Custom Object Limit", "Bypass", "Removes limits on custom editor prefabs.");
        addToggle("zoom_bypass", "Editor Zoom Bypass", "Bypass", "Allows unlimited zoom in and zoom out inside the level editor.");
        addToggle("free_scroll", "Editor Free Scroll", "Bypass", "Unlocks editor camera boundaries.");
        addToggle("song_bypass", "Custom Song Bypass", "Bypass", "Allows using unlisted or blocked audio IDs.");
        addToggle("char_filter", "Text Length & Filter Bypass", "Bypass", "Removes character limits and profanity filters in text inputs.");
        addToggle("slider_limit", "Slider Limit Bypass", "Bypass", "Allows typing values beyond standard editor slider caps.");
        addToggle("main_level_bypass", "Unlock Main Demons", "Bypass", "Unlocks Clubstep, TOE2, and Deadlocked without secret coins.");
        addToggle("vault_bypass", "Unlock All Vaults", "Bypass", "Grants immediate access to all Secret Vaults and Treasure Room.");
        addToggle("no_transition", "Instant Menu Transitions", "Bypass", "Removes fade transitions between UI scenes.");
    }

    bool getBool(const std::string& id) {
        return boolStates[id];
    }

    void setBool(const std::string& id, bool val) {
        boolStates[id] = val;
        Mod::get()->setSavedValue<bool>(id, val);
    }

    float getFloat(const std::string& id) {
        return floatStates[id];
    }

    void setFloat(const std::string& id, float val) {
        floatStates[id] = val;
        Mod::get()->setSavedValue<float>(id, val);
    }

    bool isCheating() {
        return getBool("noclip") || getBool("jump_hack") || getBool("no_spike") ||
               getBool("no_solid") || getBool("instant_complete") || getBool("hitbox_multiplier") ||
               (getBool("speedhack_en") && std::abs(getFloat("speed_val") - 1.0f) > 0.01f);
    }
};
