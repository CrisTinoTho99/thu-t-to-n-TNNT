#pragma once

#include <chrono>

namespace vlt {

// Tracks wall-clock delta time and a smoothed FPS estimate.
class Time {
public:
    Time();

    // Call once per frame; must be called before deltaSeconds()/fps().
    void tick();

    double deltaSeconds() const { return m_deltaSeconds; }
    double elapsedSeconds() const { return m_elapsedSeconds; }
    double fps() const { return m_fps; }

private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point m_lastTick;
    double m_deltaSeconds = 0.0;
    double m_elapsedSeconds = 0.0;

    double m_fpsAccumulator = 0.0;
    int m_fpsFrameCount = 0;
    double m_fps = 0.0;
};

}  // namespace vlt
