#pragma once

#include <glm/glm.hpp>

namespace vlt {

class Input;

// Minimal free-fly camera shared by every map scene.
// Convention (docs/MAP_SCHEMA.md): metres, Y up, -Z forward, yaw in degrees
// with 0 looking down -Z and positive yaw turning left; pitch is clamped to
// [-89, 89].
class Camera {
public:
    Camera() = default;

    void setPosition(const glm::vec3& position) { m_position = position; }
    void setYawPitch(float yawDegrees, float pitchDegrees);
    void setAspectRatio(float aspectRatio);
    void setMoveSpeed(float metresPerSecond) { m_moveSpeed = metresPerSecond; }

    // WASD moves along the view direction, Q/E moves vertically; holding the
    // right mouse button and moving the mouse looks around.
    void update(const Input& input, double deltaSeconds,
                bool allowMouseLook = true);

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;
    glm::vec3 forward() const;
    glm::vec3 position() const { return m_position; }
    float yawDegrees() const { return m_yaw; }
    float pitchDegrees() const { return m_pitch; }

private:
    glm::vec3 right() const;

    glm::vec3 m_position{0.0f, 1.7f, 5.0f};
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_moveSpeed = 8.0f;
    float m_fieldOfView = 60.0f;

    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_looking = false;
};

}  // namespace vlt
