#include "ui/ImGuiLayer.h"

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "core/Log.h"

namespace vlt {

ImGuiLayer::~ImGuiLayer() {
    if (m_openglInitialized) {
        ImGui_ImplOpenGL3_Shutdown();
        m_openglInitialized = false;
    }
    if (m_glfwInitialized) {
        ImGui_ImplGlfw_Shutdown();
        m_glfwInitialized = false;
    }
    if (m_contextCreated) {
        ImGui::DestroyContext();
        m_contextCreated = false;
    }
}

void ImGuiLayer::initialize(GLFWwindow* window) {
    if (window == nullptr) {
        throw std::runtime_error("Cannot initialize ImGui without a window.");
    }
    if (m_contextCreated) {
        throw std::runtime_error("ImGui layer is already initialized.");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_contextCreated = true;
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

#if defined(_WIN32)
    const char* windowsDirectory = std::getenv("WINDIR");
    if (windowsDirectory != nullptr) {
        const std::string fontPath =
            (std::filesystem::path(windowsDirectory) / "Fonts" / "segoeui.ttf")
                .string();
        if (io.Fonts->AddFontFromFileTTF(
                fontPath.c_str(), 16.0f, nullptr,
                io.Fonts->GetGlyphRangesVietnamese()) == nullptr) {
            VLT_LOG_WARN(
                "Could not load Segoe UI from '%s'; Vietnamese text may not "
                "render correctly.",
                fontPath.c_str());
        }
    } else {
        VLT_LOG_WARN(
            "WINDIR is unset; Vietnamese text may not render correctly.");
    }
#endif

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        throw std::runtime_error("Failed to initialize the ImGui GLFW backend.");
    }
    m_glfwInitialized = true;
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        throw std::runtime_error(
            "Failed to initialize the ImGui OpenGL 3 backend.");
    }
    m_openglInitialized = true;
}

void ImGuiLayer::newFrame() {
    if (!m_openglInitialized || !m_glfwInitialized) {
        return;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render() {
    if (!m_openglInitialized) {
        return;
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiLayer::wantsMouseCapture() const {
    return m_contextCreated && ImGui::GetIO().WantCaptureMouse;
}

}  // namespace vlt
