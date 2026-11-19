#include "graphics/ShaderProgram.h"

#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <string>

#include "core/Log.h"
#include "graphics/GL.h"

namespace vlt {
namespace {

std::string shaderInfoLog(unsigned int shader) {
    int length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return "(no shader info log)";
    }
    std::string log(static_cast<std::size_t>(length), '\0');
    int written = 0;
    glGetShaderInfoLog(shader, length, &written, log.data());
    log.resize(static_cast<std::size_t>(written > 0 ? written : 0));
    return log;
}

std::string programInfoLog(unsigned int program) {
    int length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return "(no program info log)";
    }
    std::string log(static_cast<std::size_t>(length), '\0');
    int written = 0;
    glGetProgramInfoLog(program, length, &written, log.data());
    log.resize(static_cast<std::size_t>(written > 0 ? written : 0));
    return log;
}

unsigned int compileShader(unsigned int type, const char* source,
                           const char* label) {
    const unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        const std::string log = shaderInfoLog(shader);
        glDeleteShader(shader);
        VLT_LOG_ERROR("%s shader compilation failed: %s", label, log.c_str());
        throw std::runtime_error(std::string(label) +
                                 " shader compilation failed: " + log);
    }
    return shader;
}

}  // namespace

ShaderProgram::ShaderProgram(const char* vertexSource,
                             const char* fragmentSource) {
    const unsigned int vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexSource, "Vertex");
    const unsigned int fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentSource, "Fragment");

    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int linked = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        const std::string log = programInfoLog(m_program);
        glDeleteProgram(m_program);
        m_program = 0;
        VLT_LOG_ERROR("Shader program link failed: %s", log.c_str());
        throw std::runtime_error("Shader program link failed: " + log);
    }
}

ShaderProgram::~ShaderProgram() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

void ShaderProgram::use() const {
    glUseProgram(m_program);
}

void ShaderProgram::setMat4(const char* uniformName,
                            const glm::mat4& value) const {
    const int location = glGetUniformLocation(m_program, uniformName);
    if (location < 0) {
        VLT_LOG_WARN("Shader uniform '%s' not found.", uniformName);
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setMat3(const char* uniformName,
                            const glm::mat3& value) const {
    const int location = glGetUniformLocation(m_program, uniformName);
    if (location < 0) {
        VLT_LOG_WARN("Shader uniform '%s' not found.", uniformName);
        return;
    }
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::setVec3(const char* uniformName,
                            const glm::vec3& value) const {
    const int location = glGetUniformLocation(m_program, uniformName);
    if (location < 0) {
        VLT_LOG_WARN("Shader uniform '%s' not found.", uniformName);
        return;
    }
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setInt(const char* uniformName, int value) const {
    const int location = glGetUniformLocation(m_program, uniformName);
    if (location < 0) {
        VLT_LOG_WARN("Shader uniform '%s' not found.", uniformName);
        return;
    }
    glUniform1i(location, value);
}

}  // namespace vlt
