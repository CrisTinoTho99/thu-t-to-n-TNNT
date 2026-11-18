#include "core/Window.h"

#include <stdexcept>
#include <utility>

#include "core/Input.h"
#include "core/Log.h"
#include "graphics/GL.h"

namespace vlt {
namespace {

std::string glfwErrorString() {
    const char* description = nullptr;
    const int code = glfwGetError(&description);
    if (description != nullptr) {
        return std::string("GLFW error ") + std::to_string(code) + ": " +
               description;
    }
    return "GLFW error " + std::to_string(code) + " (no description)";
}

}  // namespace

Window::Window(int width, int height, const std::string& title, Input& input)
    : m_input(input) {
    glfwSetErrorCallback(&Window::errorCallback);

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW. " +
                                 glfwErrorString());
    }
    m_glfwInitialized = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (m_handle == nullptr) {
        const std::string error = glfwErrorString();
        glfwTerminate();
        m_glfwInitialized = false;
        throw std::runtime_error("Failed to create the application window. " +
                                 error);
    }

    glfwSetWindowUserPointer(m_handle, this);
    glfwSetKeyCallback(m_handle, &Window::keyCallback);
    glfwSetCursorPosCallback(m_handle, &Window::cursorPosCallback);
    glfwSetMouseButtonCallback(m_handle, &Window::mouseButtonCallback);
    glfwSetScrollCallback(m_handle, &Window::scrollCallback);
    glfwSetFramebufferSizeCallback(m_handle, &Window::framebufferSizeCallback);

    glfwMakeContextCurrent(m_handle);
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
        glfwTerminate();
        m_glfwInitialized = false;
        throw std::runtime_error(
            "Failed to load OpenGL functions with GLAD (3.3 core context).");
    }
    glfwSwapInterval(1);
    glfwGetFramebufferSize(m_handle, &m_framebufferWidth, &m_framebufferHeight);

    VLT_LOG_INFO("Window created: %dx%d, OpenGL %s", width, height,
                 reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    VLT_LOG_INFO("Renderer: %s",
                 reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
}

Window::~Window() {
    if (m_handle != nullptr) {
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
    }
    if (m_glfwInitialized) {
        glfwTerminate();
        m_glfwInitialized = false;
    }
}

bool Window::shouldClose() const {
    return m_handle == nullptr || glfwWindowShouldClose(m_handle) == GLFW_TRUE;
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    if (m_handle != nullptr) {
        glfwSwapBuffers(m_handle);
    }
}

void Window::setTitle(const std::string& title) {
    if (m_handle != nullptr) {
        glfwSetWindowTitle(m_handle, title.c_str());
    }
}

void Window::setFramebufferSizeCallback(FramebufferSizeCallback callback) {
    m_framebufferSizeCallback = std::move(callback);
}

void Window::errorCallback(int code, const char* description) {
    VLT_LOG_WARN("GLFW error %d: %s", code,
                 description != nullptr ? description : "(no description)");
}

void Window::keyCallback(GLFWwindow* window, int key, int /*scancode*/,
                         int action, int mods) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        self->m_input.onKey(key, action, mods);
    }
}

void Window::cursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        self->m_input.onCursorPos(x, y);
    }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action,
                                 int /*mods*/) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        self->m_input.onMouseButton(button, action);
    }
}

void Window::scrollCallback(GLFWwindow* window, double /*xOffset*/,
                            double yOffset) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        self->m_input.onScroll(yOffset);
    }
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self == nullptr) {
        return;
    }
    self->m_framebufferWidth = width;
    self->m_framebufferHeight = height;
    if (self->m_framebufferSizeCallback) {
        self->m_framebufferSizeCallback(width, height);
    }
}

}  // namespace vlt
