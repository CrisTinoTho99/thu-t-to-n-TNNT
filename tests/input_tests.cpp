#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "core/Input.h"

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& label) {
    ++g_checks;
    if (condition) {
        std::cout << "[PASS] " << label << '\n';
        return;
    }
    ++g_failures;
    std::cout << "[FAIL] " << label << '\n';
}

}  // namespace

int main() {
    vlt::Input input;
    input.onMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
    check(input.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT),
          "mouse press records held state");
    check(input.wasMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT),
          "mouse press records a one-frame transition");

    input.endFrame();
    check(!input.wasMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT),
          "mouse press transition clears at end of frame");
    check(input.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT),
          "held state remains until release");

    input.onMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
    check(!input.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT),
          "mouse release clears held state");
    check(!input.wasMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT),
          "release does not synthesize a press");

    std::cout << g_checks - g_failures << "/" << g_checks
              << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
