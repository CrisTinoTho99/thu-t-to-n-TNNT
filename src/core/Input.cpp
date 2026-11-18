#include "core/Input.h"

#include <GLFW/glfw3.h>

namespace vlt {

void Input::onKey(int key, int action, int /*mods*/) {
    if (!isValidIndex(key, kKeyCount)) {
        return;
    }

    if (action == GLFW_PRESS) {
        m_keysDown[key] = true;
        m_keysPressed[key] = true;
    } else if (action == GLFW_RELEASE) {
        m_keysDown[key] = false;
        m_keysReleased[key] = true;
    }
    // GLFW_REPEAT intentionally does not count as a new press.
}

void Input::onMouseButton(int button, int action) {
    if (!isValidIndex(button, kMouseButtonCount)) {
        return;
    }
    m_mouseDown[button] = (action == GLFW_PRESS);
}

void Input::onCursorPos(double x, double y) {
    m_mouseX = x;
    m_mouseY = y;
}

void Input::onScroll(double yOffset) {
    m_scrollDelta += yOffset;
}

bool Input::isKeyDown(int key) const {
    return isValidIndex(key, kKeyCount) && m_keysDown[key];
}

bool Input::wasKeyPressed(int key) const {
    return isValidIndex(key, kKeyCount) && m_keysPressed[key];
}

bool Input::wasKeyReleased(int key) const {
    return isValidIndex(key, kKeyCount) && m_keysReleased[key];
}

bool Input::isMouseButtonDown(int button) const {
    return isValidIndex(button, kMouseButtonCount) && m_mouseDown[button];
}

void Input::endFrame() {
    m_keysPressed.fill(false);
    m_keysReleased.fill(false);
    m_scrollDelta = 0.0;
}

}  // namespace vlt
