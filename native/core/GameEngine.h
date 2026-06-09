#pragma once
#include "GameLoop.h"
#include "world/World.h"
#include "world/WorldSerializer.h"
#include "render/Renderer.h"
#include "input/InputHandler.h"
#include <android/asset_manager.h>
#include <string>

namespace minemc {

// Player state
struct Player {
    Vec3  pos{0, 70, 0};
    float yaw   = 0.0f;  // radians, 0 = +Z
    float pitch = 0.0f;  // radians, clipped to ±89°

    Vec3  vel{0, 0, 0};
    bool  onGround  = false;
    bool  noclip    = false; // debug

    // Which block type is selected for placement
    BlockType selectedBlock = BlockType::Grass;

    static constexpr float HEIGHT = 1.8f;
    static constexpr float RADIUS = 0.3f;
    static constexpr float SPEED  = 5.0f;
    static constexpr float JUMP_V = 7.0f;
    static constexpr float GRAVITY = -20.0f;

    // Eye position (camera)
    Vec3 eyePos() const { return { pos.x, pos.y + 1.6f, pos.z }; }

    // Look direction
    Vec3 forward() const;
};

class GameEngine {
public:
    GameEngine() = default;
    ~GameEngine() = default;

    bool init(AAssetManager* am, const std::string& savePath,
              int width, int height);
    void onSurfaceCreated();
    void onSurfaceChanged(int w, int h);
    void drawFrame();
    void pause();
    void resume();
    void destroy();

    // Touch input
    void onTouch(int action, int pointerId, float x, float y);

    // World management
    void newWorld();
    void saveWorld();

    // Queries
    std::string debugInfo() const;
    bool savedWorldExists() const;

private:
    GameLoop     mLoop;
    World        mWorld;
    Renderer     mRenderer;
    InputHandler mInput;
    Player       mPlayer;

    AAssetManager* mAssets    = nullptr;
    std::string    mSavePath;
    int            mWidth = 0, mHeight = 0;
    bool           mInitialized = false;

    static constexpr int RENDER_DIST = 4;

    void tick(float dt);
    void render();
    void updatePhysics(float dt, const MoveInput& move);
    void handleBlockInteraction(const BlockAction& action);
    bool raycast(Vec3& hitBlock, Vec3& hitNormal, float maxDist = 5.0f) const;

    // Collision helpers
    bool isSolidAt(float x, float y, float z) const;
    bool playerCollides(const Vec3& pos) const;
};

} // namespace minemc
