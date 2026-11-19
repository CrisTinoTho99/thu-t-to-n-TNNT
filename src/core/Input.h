#pragma once

#include <array>

namespace vlt {

// Minimal keyboard/mouse state gathered from GLFW callbacks.
// Call endFrame() once per frame, after scenes have consumed the input.
class Input {
public:
    static constexpr int kKeyCount = 512;
    static constexpr int kMouseButtonCount = 8;

    void onKey(int key, int action, int mods);
    void onMouseButton(int button, int action);
    void onCursorPos(double x, double y);
    void onScroll(double yOffset);

    bool isKeyDown(int key) const;
    bool wasKeyPressed(int key) const;
    bool wasKeyReleased(int key) const;

    bool isMouseButtonDown(int button) const;
    bool wasMouseButtonPressed(int button) const;

    double mouseX() const { return m_mouseX; }
    double mouseY() const { return m_mouseY; }
    double scrollDelta() const { return m_scrollDelta; }

    // Clears per-frame transitions (pressed/released/scroll).
    void endFrame();

private:
    static bool isValidIndex(int index, int count) {
        return index >= 0 && index < count;
    }

    std::array<bool, kKeyCount> m_keysDown{};
    std::array<bool, kKeyCount> m_keysPressed{};
    std::array<bool, kKeyCount> m_keysReleased{};

    std::array<bool, kMouseButtonCount> m_mouseDown{};
    std::array<bool, kMouseButtonCount> m_mousePressed{};

    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    double m_scrollDelta = 0.0;
};

}  // namespace vlt
