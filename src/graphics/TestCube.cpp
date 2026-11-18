#include "graphics/TestCube.h"

#include "core/Log.h"
#include "graphics/GL.h"

namespace vlt {
namespace {

// position (x, y, z) + colour (r, g, b) per corner.
constexpr float kVertexData[] = {
    // clang-format off
    -1.0f, -1.0f, -1.0f,  0.90f, 0.20f, 0.20f,
     1.0f, -1.0f, -1.0f,  0.20f, 0.80f, 0.30f,
     1.0f,  1.0f, -1.0f,  0.20f, 0.40f, 0.90f,
    -1.0f,  1.0f, -1.0f,  0.90f, 0.80f, 0.20f,
    -1.0f, -1.0f,  1.0f,  0.80f, 0.20f, 0.80f,
     1.0f, -1.0f,  1.0f,  0.20f, 0.80f, 0.80f,
     1.0f,  1.0f,  1.0f,  0.95f, 0.95f, 0.95f,
    -1.0f,  1.0f,  1.0f,  0.90f, 0.50f, 0.10f,
    // clang-format on
};

constexpr unsigned int kIndexData[] = {
    0, 1, 2, 0, 2, 3,  // -Z
    4, 6, 5, 4, 7, 6,  // +Z
    0, 4, 5, 0, 5, 1,  // -Y
    3, 2, 6, 3, 6, 7,  // +Y
    0, 3, 7, 0, 7, 4,  // -X
    1, 5, 6, 1, 6, 2,  // +X
};

constexpr int kIndexCount =
    static_cast<int>(sizeof(kIndexData) / sizeof(kIndexData[0]));

}  // namespace

TestCube::TestCube() {
    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);
    glGenBuffers(1, &m_indexBuffer);

    glBindVertexArray(m_vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertexData), kVertexData,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndexData), kIndexData,
                 GL_STATIC_DRAW);

    constexpr int kStride = 6 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kStride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, kStride,
                          reinterpret_cast<const void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    const GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        VLT_LOG_WARN("OpenGL error while creating the test cube: 0x%04X",
                     static_cast<unsigned int>(error));
    }
}

TestCube::~TestCube() {
    if (m_indexBuffer != 0) {
        glDeleteBuffers(1, &m_indexBuffer);
    }
    if (m_vertexBuffer != 0) {
        glDeleteBuffers(1, &m_vertexBuffer);
    }
    if (m_vertexArray != 0) {
        glDeleteVertexArrays(1, &m_vertexArray);
    }
}

void TestCube::draw() const {
    glBindVertexArray(m_vertexArray);
    glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

}  // namespace vlt
