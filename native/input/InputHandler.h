#pragma once
#include <unordered_map>
#include <cstdint>

namespace minemc {

// Screen-space touch zones (ratios)
constexpr float JOYSTICK_ZONE_X = 0.35f; // left 35% = move joystick
constexpr float JOYSTICK_RADIUS = 120.0f;

// Touch actions from Android MotionEvent
enum TouchAction {
    TOUCH_DOWN   = 0,
    TOUCH_UP     = 1,
    TOUCH_MOVE   = 2,
    TOUCH_CANCEL = 3,
    TOUCH_PTR_DOWN = 5,
    TOUCH_PTR_UP   = 6,
};

struct TouchPoint {
    float x, y;
    bool active = false;
    bool inJoystick = false;

    // For joystick tracking
    float baseX = 0, baseY = 0;
};

// Output movement vector [0,1] each axis
struct MoveInput {
    float forward = 0; // +1 forward, -1 backward
    float strafe  = 0; // +1 right,   -1 left
    bool  jump    = false;
};

// Look delta in radians (accumulated per frame, then reset)
struct LookDelta {
    float dyaw   = 0;
    float dpitch = 0;
};

// Block interaction flags (reset after each frame)
struct BlockAction {
    bool breakBlock = false;
    bool placeBlock = false;
};

class InputHandler {
public:
    InputHandler() = default;

    void init(float screenW, float screenH);
    void onTouch(int action, int pointerId, float x, float y);

    // Called each frame to read accumulated state
    MoveInput   getMoveInput()  const;
    LookDelta   consumeLookDelta();
    BlockAction consumeBlockAction();

    bool isJumpPressed() const { return mJumpPressed; }

private:
    float mScreenW = 1280, mScreenH = 720;

    // Pointer tracking (Android supports up to 10 simultaneous pointers)
    static constexpr int MAX_PTRS = 10;
    TouchPoint mPointers[MAX_PTRS]{};

    int  mJoystickPtr  = -1;  // pointer ID driving joystick
    int  mLookPtr      = -1;  // pointer ID driving camera look
    int  mJumpPtr      = -1;  // pointer ID on jump button

    float mJoystickX = 0, mJoystickY = 0; // normalised [-1,1]

    float mLookDYaw   = 0;
    float mLookDPitch = 0;

    bool mBreakBlock   = false;
    bool mPlaceBlock   = false;
    bool mJumpPressed  = false;

    // Tap detection for break/place
    float mLookStartX  = 0, mLookStartY = 0;
    float mTapThreshold = 20.0f;   // pixels
    float mLookSensX    = 0.003f;  // rad/px
    float mLookSensY    = 0.003f;

    bool isJumpZone(float x, float y) const;
    bool isJoystickZone(float x, float y) const;
};

} // namespace minemc
