#pragma once

#include <glm/glm.hpp>

namespace vlt {

// Owns a linked vertex/fragment program. Compilation and link failures are
// logged with the driver info log and thrown, so a broken shader never renders
// silently.
class ShaderProgram {
public:
    ShaderProgram(const char* vertexSource, const char* fragmentSource);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    void use() const;
    void setMat4(const char* uniformName, const glm::mat4& value) const;
    void setMat3(const char* uniformName, const glm::mat3& value) const;
    void setVec3(const char* uniformName, const glm::vec3& value) const;
    void setInt(const char* uniformName, int value) const;

private:
    unsigned int m_program = 0;
};

}  // namespace vlt
