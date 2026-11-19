#include "graphics/Model.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

#include "graphics/GL.h"
#include "graphics/ShaderProgram.h"

namespace vlt {

class Model::Mesh {
public:
    explicit Mesh(ImportedMesh mesh)
        : m_nodeName(std::move(mesh.nodeName)),
          m_nodeTransform(mesh.nodeTransform) {
        if (mesh.vertices.empty() || mesh.indices.empty()) {
            throw std::runtime_error("cannot upload an empty mesh");
        }
        if (mesh.vertices.size() % 9 != 0) {
            throw std::runtime_error("mesh vertex data has an invalid stride");
        }
        calculateWorldBounds(mesh.vertices);

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);
        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(mesh.vertices.size() *
                                             sizeof(float)),
                     mesh.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(mesh.indices.size() *
                                             sizeof(std::uint32_t)),
                     mesh.indices.data(), GL_STATIC_DRAW);

        constexpr GLsizei stride = 9 * static_cast<GLsizei>(sizeof(float));
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<const void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<const void*>(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
        m_indexCount = static_cast<GLsizei>(mesh.indices.size());
    }

    ~Mesh() {
        if (m_ebo != 0) {
            glDeleteBuffers(1, &m_ebo);
        }
        if (m_vbo != 0) {
            glDeleteBuffers(1, &m_vbo);
        }
        if (m_vao != 0) {
            glDeleteVertexArrays(1, &m_vao);
        }
    }

    void draw(const ShaderProgram& shader, const glm::mat4& viewProjection,
              const glm::vec3& lightDirection,
              const std::string& highlightedNode) const {
        shader.setMat4("uMvp", viewProjection * m_nodeTransform);
        shader.setMat3(
            "uNormalMatrix",
            glm::transpose(glm::inverse(glm::mat3(m_nodeTransform))));
        shader.setVec3("uLightDirection", lightDirection);
        shader.setInt("uHighlighted", m_nodeName == highlightedNode ? 1 : 0);
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }

    std::optional<float> pick(const Ray& ray) const {
        return intersectRayAabb(ray, m_worldBoundsMin, m_worldBoundsMax);
    }

    const std::string& nodeName() const { return m_nodeName; }

private:
    void calculateWorldBounds(const std::vector<float>& vertices) {
        glm::vec3 localMin(std::numeric_limits<float>::max());
        glm::vec3 localMax(std::numeric_limits<float>::lowest());
        for (std::size_t offset = 0; offset < vertices.size(); offset += 9) {
            const glm::vec3 position(vertices[offset], vertices[offset + 1],
                                     vertices[offset + 2]);
            localMin = glm::min(localMin, position);
            localMax = glm::max(localMax, position);
        }

        m_worldBoundsMin = glm::vec3(std::numeric_limits<float>::max());
        m_worldBoundsMax = glm::vec3(std::numeric_limits<float>::lowest());
        for (int mask = 0; mask < 8; ++mask) {
            const glm::vec3 corner(
                (mask & 1) != 0 ? localMax.x : localMin.x,
                (mask & 2) != 0 ? localMax.y : localMin.y,
                (mask & 4) != 0 ? localMax.z : localMin.z);
            const glm::vec3 world =
                glm::vec3(m_nodeTransform * glm::vec4(corner, 1.0f));
            m_worldBoundsMin = glm::min(m_worldBoundsMin, world);
            m_worldBoundsMax = glm::max(m_worldBoundsMax, world);
        }
    }

    std::string m_nodeName;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLsizei m_indexCount = 0;
    glm::mat4 m_nodeTransform{1.0f};
    glm::vec3 m_worldBoundsMin{0.0f};
    glm::vec3 m_worldBoundsMax{0.0f};
};

Model::Model(ImportedModel model) {
    m_meshes.reserve(model.meshes.size());
    for (ImportedMesh& mesh : model.meshes) {
        m_meshes.push_back(std::make_unique<Mesh>(std::move(mesh)));
    }
}

Model::~Model() = default;

void Model::draw(const ShaderProgram& shader,
                 const glm::mat4& viewProjection,
                 const glm::vec3& lightDirection,
                 const std::string& highlightedNode) const {
    shader.use();
    for (const std::unique_ptr<Mesh>& mesh : m_meshes) {
        mesh->draw(shader, viewProjection, lightDirection, highlightedNode);
    }
    glBindVertexArray(0);
}

std::optional<ModelHit> Model::pick(const Ray& ray) const {
    std::optional<ModelHit> nearest;
    for (const std::unique_ptr<Mesh>& mesh : m_meshes) {
        const std::optional<float> distance = mesh->pick(ray);
        if (distance && (!nearest || *distance < nearest->distance)) {
            nearest = ModelHit{mesh->nodeName(), *distance};
        }
    }
    return nearest;
}

}  // namespace vlt
