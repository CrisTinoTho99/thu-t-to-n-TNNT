#include "graphics/HotspotRenderer.h"

#include <cstddef>
#include <vector>

#include "graphics/GL.h"
#include "graphics/ShaderProgram.h"

namespace vlt {
namespace {

constexpr const char* kVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPosition;
uniform mat4 uMvp;
void main() {
    gl_Position = uMvp * vec4(aPosition, 1.0);
    gl_PointSize = 14.0;
}
)";

constexpr const char* kFragmentShader = R"(#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main() {
    vec2 centered = gl_PointCoord - vec2(0.5);
    if (dot(centered, centered) > 0.25) {
        discard;
    }
    FragColor = vec4(uColor, 1.0);
}
)";

}  // namespace

HotspotRenderer::HotspotRenderer(const std::vector<MapHotspot>& hotspots) {
    if (hotspots.empty()) {
        return;
    }

    std::vector<float> positions;
    positions.reserve(hotspots.size() * 3);
    for (const MapHotspot& hotspot : hotspots) {
        positions.insert(positions.end(),
                         {hotspot.position.x, hotspot.position.y,
                          hotspot.position.z});
    }

    m_shader = std::make_unique<ShaderProgram>(kVertexShader, kFragmentShader);
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(positions.size() * sizeof(float)),
                 positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * static_cast<GLsizei>(sizeof(float)), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    m_count = static_cast<int>(hotspots.size());
}

HotspotRenderer::~HotspotRenderer() {
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
}

void HotspotRenderer::draw(const glm::mat4& viewProjection,
                           std::size_t selectedIndex) const {
    if (m_shader == nullptr || m_count == 0) {
        return;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    m_shader->use();
    m_shader->setMat4("uMvp", viewProjection);
    m_shader->setVec3("uColor", glm::vec3(1.0f, 0.55f, 0.12f));
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, m_count);

    if (selectedIndex < static_cast<std::size_t>(m_count)) {
        m_shader->setVec3("uColor", glm::vec3(1.0f, 0.92f, 0.28f));
        glDrawArrays(GL_POINTS, static_cast<GLint>(selectedIndex), 1);
    }
    glBindVertexArray(0);
}

}  // namespace vlt
