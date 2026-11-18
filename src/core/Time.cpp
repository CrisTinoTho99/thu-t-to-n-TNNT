#include "core/Time.h"

namespace vlt {
namespace {
// A stalled frame (window drag, debugger break) should not produce a huge
// simulation step.
constexpr double kMaxDeltaSeconds = 0.25;
}  // namespace

Time::Time() : m_lastTick(Clock::now()) {}

void Time::tick() {
    const Clock::time_point now = Clock::now();
    const double rawDelta =
        std::chrono::duration<double>(now - m_lastTick).count();
    m_lastTick = now;

    m_deltaSeconds = rawDelta > kMaxDeltaSeconds ? kMaxDeltaSeconds : rawDelta;
    m_elapsedSeconds += m_deltaSeconds;

    m_fpsAccumulator += rawDelta;
    ++m_fpsFrameCount;
    if (m_fpsAccumulator >= 0.5) {
        m_fps = static_cast<double>(m_fpsFrameCount) / m_fpsAccumulator;
        m_fpsAccumulator = 0.0;
        m_fpsFrameCount = 0;
    }
}

}  // namespace vlt
