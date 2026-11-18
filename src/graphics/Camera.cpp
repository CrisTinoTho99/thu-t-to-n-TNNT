#include "graphics/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

#include "core/Input.h"
#include "graphics/GL.h"

namespace vlt {
namespace {
constexpr float kMouseSensitivity = 0.1f;  // degrees per pixel
constexpr float kMinPitch = -89.0f;
constexpr float kMaxPitch = 89.0f;

float clampPitch(float pitch) {
    return std::clamp(pitch, kMinPitch, kMaxPitch);
}
}  // namespace

void Camera::setYawPitch(float yawDegrees, float pitchDegrees) {
    m_yaw = yawDegrees;
    m_pitch = clampPitch(pitchDegrees);
}

void Camera::setAspectRatio(float aspectRatio) {
    if (aspectRatio > 0.0f) {
        m_aspectRatio = aspectRatio;
    }
}

glm::vec3 Camera::forward() const {
    const float yawRadians = glm::radians(m_yaw);
    const float pitchRadians = glm::radians(m_pitch);
    const float cosPitch = std::cos(pitchRadians);
    return glm::normalize(glm::vec3(-std::sin(yawRadians) * cosPitch,
                                    std::sin(pitchRadians),
                                    -std::cos(yawRadians) * cosPitch));
}

glm::vec3 Camera::right() const {
    return glm::normalize(glm::cross(forward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

void Camera::update(const Input& input, double deltaSeconds) {
    if (input.isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!m_looking) {
            // Start from the current cursor position so the view does not jump.
            m_looking = true;
            m_lastMouseX = input.mouseX();
            m_lastMouseY = input.mouseY();
        } else {
            const double dx = input.mouseX() - m_lastMouseX;
            const double dy = input.mouseY() - m_lastMouseY;
            m_lastMouseX = input.mouseX();
            m_lastMouseY = input.mouseY();

            m_yaw -= static_cast<float>(dx) * kMouseSensitivity;
            m_pitch = clampPitch(m_pitch - static_cast<float>(dy) * kMouseSensitivity);
        }
    } else {
        m_looking = false;
    }

    const float dt = static_cast<float>(deltaSeconds);
    float speed = m_moveSpeed;
    if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT) || input.isKeyDown(GLFW_KEY_RIGHT_SHIFT)) {
        speed *= 3.0f;
    }

    const glm::vec3 forwardVector = forward();
    const glm::vec3 rightVector = right();
    glm::vec3 movement(0.0f);
    if (input.isKeyDown(GLFW_KEY_W)) movement += forwardVector;
    if (input.isKeyDown(GLFW_KEY_S)) movement -= forwardVector;
    if (input.isKeyDown(GLFW_KEY_D)) movement += rightVector;
    if (input.isKeyDown(GLFW_KEY_A)) movement -= rightVector;
    if (input.isKeyDown(GLFW_KEY_E)) movement += glm::vec3(0.0f, 1.0f, 0.0f);
    if (input.isKeyDown(GLFW_KEY_Q)) movement -= glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::length(movement) > 0.0f) {
        m_position += glm::normalize(movement) * speed * dt;
    }
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(m_position, m_position + forward(),
                       glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::projectionMatrix() const {
    return glm::perspective(glm::radians(m_fieldOfView), m_aspectRatio, 0.1f, 5000.0f);
}

}  // namespace vlt
