#pragma once

#include <functional>
#include <string>

struct GLFWwindow;

namespace vlt {

class Input;

// Owns GLFW initialization, the single application window and its OpenGL 3.3
// core context. Callbacks are forwarded to Input; framebuffer size changes are
// forwarded to the registered handler so the viewport can follow.
class Window {
public:
    using FramebufferSizeCallback = std::function<void(int, int)>;

    Window(int width, int height, const std::string& title, Input& input);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    void setTitle(const std::string& title);

    int framebufferWidth() const { return m_framebufferWidth; }
    int framebufferHeight() const { return m_framebufferHeight; }

    void setFramebufferSizeCallback(FramebufferSizeCallback callback);

    GLFWwindow* handle() const { return m_handle; }

private:
    static void errorCallback(int code, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode,
                            int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action,
                                    int mods);
    static void scrollCallback(GLFWwindow* window, double xOffset,
                               double yOffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width,
                                        int height);

    GLFWwindow* m_handle = nullptr;
    Input& m_input;
    FramebufferSizeCallback m_framebufferSizeCallback;
    int m_framebufferWidth = 0;
    int m_framebufferHeight = 0;
    bool m_glfwInitialized = false;
};

}  // namespace vlt
