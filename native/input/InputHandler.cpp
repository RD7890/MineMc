#include "InputHandler.h"
#include <cmath>
#include <android/log.h>
#define TAG "MineMc/Input"

namespace minemc {

void InputHandler::init(float screenW, float screenH) {
    mScreenW = screenW;
    mScreenH = screenH;
}

bool InputHandler::isJoystickZone(float x, float /*y*/) const {
    return x < mScreenW * JOYSTICK_ZONE_X;
}

bool InputHandler::isJumpZone(float x, float y) const {
    // Bottom-right corner 15% × 20% of screen
    return x > mScreenW * 0.85f && y > mScreenH * 0.7f;
}

void InputHandler::onTouch(int action, int pointerId, float x, float y) {
    if (pointerId < 0 || pointerId >= MAX_PTRS) return;

    bool isDown = (action == TOUCH_DOWN || action == TOUCH_PTR_DOWN);
    bool isUp   = (action == TOUCH_UP   || action == TOUCH_PTR_UP || action == TOUCH_CANCEL);
    bool isMove = (action == TOUCH_MOVE);

    if (isDown) {
        mPointers[pointerId] = { x, y, true, false, x, y };

        if (isJoystickZone(x, y) && mJoystickPtr == -1) {
            mJoystickPtr = pointerId;
            mPointers[pointerId].inJoystick = true;
            mPointers[pointerId].baseX = x;
            mPointers[pointerId].baseY = y;
            mJoystickX = 0;
            mJoystickY = 0;
        } else if (isJumpZone(x, y)) {
            mJumpPtr = pointerId;
            mJumpPressed = true;
        } else if (mLookPtr == -1) {
            mLookPtr = pointerId;
            mLookStartX = x;
            mLookStartY = y;
        }
    }

    if (isMove) {
        if (pointerId == mJoystickPtr) {
            float dx = x - mPointers[pointerId].baseX;
            float dy = y - mPointers[pointerId].baseY;
            float len = std::sqrt(dx*dx + dy*dy);
            if (len > JOYSTICK_RADIUS) {
                dx = dx / len * JOYSTICK_RADIUS;
                dy = dy / len * JOYSTICK_RADIUS;
                len = JOYSTICK_RADIUS;
            }
            mJoystickX = dx / JOYSTICK_RADIUS;
            mJoystickY = dy / JOYSTICK_RADIUS;
        } else if (pointerId == mLookPtr) {
            float ddx = x - mPointers[pointerId].x;
            float ddy = y - mPointers[pointerId].y;
            mLookDYaw   += ddx * mLookSensX;
            mLookDPitch += ddy * mLookSensY;
        }

        if (mPointers[pointerId].active) {
            mPointers[pointerId].x = x;
            mPointers[pointerId].y = y;
        }
    }

    if (isUp) {
        if (pointerId == mJoystickPtr) {
            mJoystickPtr = -1;
            mJoystickX = 0;
            mJoystickY = 0;
        } else if (pointerId == mLookPtr) {
            // Check for tap (short movement = block interaction)
            float totalDx = x - mLookStartX;
            float totalDy = y - mLookStartY;
            float dist = std::sqrt(totalDx*totalDx + totalDy*totalDy);
            if (dist < mTapThreshold) {
                // Left side of right zone = break; right side = place
                if (x < mScreenW * 0.65f) mBreakBlock = true;
                else                       mPlaceBlock = true;
            }
            mLookPtr = -1;
        } else if (pointerId == mJumpPtr) {
            mJumpPressed = false;
            mJumpPtr = -1;
        }

        if (pointerId < MAX_PTRS) {
            mPointers[pointerId].active = false;
            mPointers[pointerId].inJoystick = false;
        }
    }
}

MoveInput InputHandler::getMoveInput() const {
    MoveInput m;
    // Joystick Y → forward/backward; X → strafe
    m.forward = -mJoystickY; // screen Y is inverted vs forward
    m.strafe  =  mJoystickX;
    m.jump    =  mJumpPressed;
    return m;
}

LookDelta InputHandler::consumeLookDelta() {
    LookDelta d{ mLookDYaw, mLookDPitch };
    mLookDYaw = 0;
    mLookDPitch = 0;
    return d;
}

BlockAction InputHandler::consumeBlockAction() {
    BlockAction a{ mBreakBlock, mPlaceBlock };
    mBreakBlock = false;
    mPlaceBlock = false;
    return a;
}

} // namespace minemc
