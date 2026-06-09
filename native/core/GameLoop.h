#pragma once
#include <cstdint>
#include <functional>

namespace minemc {

// Fixed-timestep game loop.
// tick() is called each time step advances; render() once per frame.
class GameLoop {
public:
    using TickFn   = std::function<void(float dt)>;
    using RenderFn = std::function<void()>;

    static constexpr float TICK_RATE = 20.0f; // ticks per second
    static constexpr float TICK_DT   = 1.0f / TICK_RATE;

    void init();
    void step(const TickFn& tick, const RenderFn& render);

    float fps()  const { return mFps; }
    void pause()       { mPaused = true; }
    void resume()      { mPaused = false; }

private:
    bool     mPaused     = false;
    int64_t  mLastTimeNs = 0;
    float    mAccum      = 0.0f;

    // FPS tracking
    float   mFps        = 0.0f;
    float   mFpsAccum   = 0.0f;
    int     mFpsFrames  = 0;

    static int64_t nowNs();
};

} // namespace minemc
