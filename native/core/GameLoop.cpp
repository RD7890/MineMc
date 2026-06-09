#include "GameLoop.h"
#include <time.h>
#include <algorithm>

namespace minemc {

int64_t GameLoop::nowNs() {
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1'000'000'000LL + ts.tv_nsec;
}

void GameLoop::init() {
    mLastTimeNs = nowNs();
    mAccum      = 0.0f;
    mFps        = 0.0f;
    mFpsAccum   = 0.0f;
    mFpsFrames  = 0;
    mPaused     = false;
}

void GameLoop::step(const TickFn& tick, const RenderFn& render) {
    if (mPaused) {
        mLastTimeNs = nowNs(); // reset so we don't accumulate while paused
        return;
    }

    int64_t now = nowNs();
    float elapsed = (float)(now - mLastTimeNs) * 1e-9f;
    mLastTimeNs = now;

    // Clamp to avoid spiral of death after hiccups
    elapsed = std::min(elapsed, 0.25f);
    mAccum += elapsed;

    while (mAccum >= TICK_DT) {
        tick(TICK_DT);
        mAccum -= TICK_DT;
    }

    render();

    // FPS tracking
    mFpsAccum  += elapsed;
    mFpsFrames += 1;
    if (mFpsAccum >= 1.0f) {
        mFps       = mFpsFrames / mFpsAccum;
        mFpsAccum  = 0.0f;
        mFpsFrames = 0;
    }
}

} // namespace minemc
