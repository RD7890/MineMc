#include "GameEngine.h"
#include "world/WorldGen.h"
#include <android/log.h>
#include <cmath>
#include <sstream>
#include <iomanip>
#define TAG "MineMc/Engine"

namespace minemc {

static constexpr float PI = 3.14159265f;
static constexpr float DEG2RAD = PI / 180.0f;

Vec3 Player::forward() const {
    float cy = std::cos(yaw), sy = std::sin(yaw);
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    return { cy * cp, sp, sy * cp };
}

bool GameEngine::init(AAssetManager* am, const std::string& savePath,
                       int width, int height)
{
    mAssets   = am;
    mSavePath = savePath;
    mWidth    = width;
    mHeight   = height;

    // Try to load an existing world; else generate on first draw
    if (WorldSerializer::fileExists(savePath)) {
        if (!WorldSerializer::load(mWorld, savePath)) {
            __android_log_print(ANDROID_LOG_WARN, TAG, "Load failed, fresh world");
        }
    }

    mInput.init((float)width, (float)height);

    // Spawn player above terrain
    mPlayer.pos = { 8.0f, 80.0f, 8.0f };
    mPlayer.vel = { 0, 0, 0 };

    mLoop.init();
    mInitialized = true;
    __android_log_print(ANDROID_LOG_INFO, TAG, "Engine init OK");
    return true;
}

void GameEngine::onSurfaceCreated() {
    if (!mRenderer.init(mAssets, mWidth, mHeight)) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Renderer init failed");
    }
    mWorld.updateChunks(mPlayer.pos.x, mPlayer.pos.z, RENDER_DIST);
    mRenderer.uploadDirtyChunks(mWorld);
}

void GameEngine::onSurfaceChanged(int w, int h) {
    mWidth = w; mHeight = h;
    mRenderer.onSizeChanged(w, h);
    mInput.init((float)w, (float)h);
}

void GameEngine::drawFrame() {
    if (!mInitialized) return;
    mLoop.step(
        [this](float dt) { tick(dt); },
        [this]()         { render(); }
    );
}

void GameEngine::pause() {
    mLoop.pause();
    WorldSerializer::save(mWorld, mSavePath);
}

void GameEngine::resume() {
    mLoop.resume();
}

void GameEngine::destroy() {
    WorldSerializer::save(mWorld, mSavePath);
    mRenderer.destroy();
    mInitialized = false;
}

void GameEngine::onTouch(int action, int pointerId, float x, float y) {
    mInput.onTouch(action, pointerId, x, y);
}

void GameEngine::newWorld() {
    mWorld.clear();
    mPlayer.pos = { 8.0f, 80.0f, 8.0f };
    mPlayer.vel = { 0, 0, 0 };
    mPlayer.yaw = 0;
    mPlayer.pitch = 0;
    // Delete save file
    std::remove(mSavePath.c_str());
    mWorld.updateChunks(mPlayer.pos.x, mPlayer.pos.z, RENDER_DIST);
    mRenderer.uploadDirtyChunks(mWorld);
}

void GameEngine::saveWorld() {
    WorldSerializer::save(mWorld, mSavePath);
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void GameEngine::tick(float dt) {
    MoveInput   move   = mInput.getMoveInput();
    LookDelta   look   = mInput.consumeLookDelta();
    BlockAction action = mInput.consumeBlockAction();

    // Update camera
    mPlayer.yaw   += look.dyaw;
    mPlayer.pitch -= look.dpitch;
    mPlayer.pitch  = std::max(-89.0f * DEG2RAD,
                     std::min( 89.0f * DEG2RAD, mPlayer.pitch));

    updatePhysics(dt, move);
    handleBlockInteraction(action);

    // Keep chunks loaded around player
    mWorld.updateChunks(mPlayer.pos.x, mPlayer.pos.z, RENDER_DIST);
    mRenderer.uploadDirtyChunks(mWorld);
}

void GameEngine::updatePhysics(float dt, const MoveInput& move) {
    // Horizontal movement — use yaw to build planar forward/right vectors
    float cy = std::cos(mPlayer.yaw), sy = std::sin(mPlayer.yaw);
    float fwdX = cy, fwdZ = sy;
    float rgtX = -sy, rgtZ = cy;

    float speed = Player::SPEED;
    float dx = (fwdX * move.forward + rgtX * move.strafe) * speed;
    float dz = (fwdZ * move.forward + rgtZ * move.strafe) * speed;

    // Gravity
    if (!mPlayer.onGround) {
        mPlayer.vel.y += Player::GRAVITY * dt;
    }

    // Jump
    if (move.jump && mPlayer.onGround) {
        mPlayer.vel.y = Player::JUMP_V;
        mPlayer.onGround = false;
    }

    // Try X movement
    Vec3 tryPos = mPlayer.pos;
    tryPos.x += dx * dt;
    if (!playerCollides(tryPos)) {
        mPlayer.pos.x = tryPos.x;
    }

    // Try Z movement
    tryPos = mPlayer.pos;
    tryPos.z += dz * dt;
    if (!playerCollides(tryPos)) {
        mPlayer.pos.z = tryPos.z;
    }

    // Try Y movement
    tryPos = mPlayer.pos;
    tryPos.y += mPlayer.vel.y * dt;
    if (!playerCollides(tryPos)) {
        mPlayer.pos.y = tryPos.y;
        mPlayer.onGround = false;
    } else {
        // Landing
        if (mPlayer.vel.y < 0) {
            mPlayer.onGround = true;
        }
        mPlayer.vel.y = 0;
    }

    // Clamp to world bottom
    if (mPlayer.pos.y < 0.0f) {
        mPlayer.pos.y = 0.0f;
        mPlayer.vel.y = 0;
        mPlayer.onGround = true;
    }
}

bool GameEngine::isSolidAt(float x, float y, float z) const {
    int bx = (int)std::floor(x);
    int by = (int)std::floor(y);
    int bz = (int)std::floor(z);
    return minemc::isSolid(mWorld.getBlock(bx, by, bz));
}

bool GameEngine::playerCollides(const Vec3& pos) const {
    float r = Player::RADIUS;
    float h = Player::HEIGHT;
    // Sample 8 corners of AABB
    for (float dx : {-r, r}) {
        for (float dz : {-r, r}) {
            if (isSolidAt(pos.x + dx, pos.y,       pos.z + dz)) return true;
            if (isSolidAt(pos.x + dx, pos.y + h/2, pos.z + dz)) return true;
            if (isSolidAt(pos.x + dx, pos.y + h,   pos.z + dz)) return true;
        }
    }
    return false;
}

void GameEngine::handleBlockInteraction(const BlockAction& action) {
    if (!action.breakBlock && !action.placeBlock) return;

    Vec3 hitBlock, hitNormal;
    if (!raycast(hitBlock, hitNormal)) return;

    if (action.breakBlock) {
        mWorld.setBlock((int)hitBlock.x, (int)hitBlock.y, (int)hitBlock.z,
                        BlockType::Air);
    } else if (action.placeBlock) {
        int px = (int)(hitBlock.x + hitNormal.x);
        int py = (int)(hitBlock.y + hitNormal.y);
        int pz = (int)(hitBlock.z + hitNormal.z);
        // Don't place inside player
        Vec3 placePos{ (float)px, (float)py, (float)pz };
        if (!playerCollides(placePos)) {
            mWorld.setBlock(px, py, pz, mPlayer.selectedBlock);
        }
    }
}

bool GameEngine::raycast(Vec3& hitBlock, Vec3& hitNormal, float maxDist) const {
    Vec3 fwd = mPlayer.forward();
    Vec3 origin = mPlayer.eyePos();

    float px = origin.x, py = origin.y, pz = origin.z;
    float dx = fwd.x, dy = fwd.y, dz = fwd.z;

    // DDA raycast
    const float STEP = 0.05f;
    float prevX = px, prevY = py, prevZ = pz;

    for (float t = 0; t < maxDist; t += STEP) {
        float cx = px + dx * t;
        float cy = py + dy * t;
        float cz = pz + dz * t;

        int bx = (int)std::floor(cx);
        int by = (int)std::floor(cy);
        int bz = (int)std::floor(cz);

        if (minemc::isSolid(mWorld.getBlock(bx, by, bz))) {
            hitBlock = { (float)bx, (float)by, (float)bz };

            // Normal from previous position
            int pbx = (int)std::floor(prevX);
            int pby = (int)std::floor(prevY);
            int pbz = (int)std::floor(prevZ);
            hitNormal = {
                (float)(pbx - bx),
                (float)(pby - by),
                (float)(pbz - bz)
            };
            return true;
        }

        prevX = cx; prevY = cy; prevZ = cz;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void GameEngine::render() {
    Vec3 eye = mPlayer.eyePos();
    mRenderer.renderFrame(mWorld, eye, mPlayer.yaw, mPlayer.pitch);
}

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

std::string GameEngine::debugInfo() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "FPS:" << (int)mLoop.fps() << "\n";
    ss << "X:" << mPlayer.pos.x
       << " Y:" << mPlayer.pos.y
       << " Z:" << mPlayer.pos.z << "\n";

    int cx = (int)mPlayer.pos.x / CHUNK_SIZE;
    int cz = (int)mPlayer.pos.z / CHUNK_SIZE;
    ss << "Chunk:" << cx << "," << cz;
    return ss.str();
}

bool GameEngine::savedWorldExists() const {
    return WorldSerializer::fileExists(mSavePath);
}

} // namespace minemc
