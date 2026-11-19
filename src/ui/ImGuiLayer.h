#pragma once

struct GLFWwindow;

namespace vlt {

class ImGuiLayer {
public:
    ImGuiLayer() = default;
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    void initialize(GLFWwindow* window);
    void newFrame();
    void render();
    bool wantsMouseCapture() const;

private:
    bool m_contextCreated = false;
    bool m_glfwInitialized = false;
    bool m_openglInitialized = false;
};

}  // namespace vlt
