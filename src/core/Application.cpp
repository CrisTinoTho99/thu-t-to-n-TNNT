#include "core/Application.h"

#include <cstdio>
#include <exception>
#include <memory>
#include <string>

#include "core/Log.h"
#include "core/Paths.h"
#include "graphics/GL.h"
#include "scenes/MainMenuScene.h"

namespace vlt {
namespace {
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
const char* kBaseTitle = "VirtualLandmarkTour";
}  // namespace

Application::Application(int argc, char** argv)
    : m_window(kWindowWidth, kWindowHeight, kBaseTitle, m_input),
      m_baseTitle(kBaseTitle) {
    const std::filesystem::path executablePath =
        (argc > 0 && argv != nullptr && argv[0] != nullptr)
            ? std::filesystem::path(argv[0])
            : std::filesystem::path();
    m_assetRoot = paths::findAssetRoot(executablePath);

    if (m_assetRoot.empty()) {
        VLT_LOG_WARN(
            "Could not locate an 'assets/maps' directory from the working "
            "directory or the executable location.");
    } else {
        VLT_LOG_INFO("Asset root: %s", m_assetRoot.string().c_str());
    }

    m_window.setFramebufferSizeCallback(
        [this](int width, int height) { onFramebufferResize(width, height); });
    onFramebufferResize(m_window.framebufferWidth(), m_window.framebufferHeight());

    m_sceneManager.requestScene(
        std::make_unique<MainMenuScene>(m_sceneManager, m_assetRoot));
}

Application::~Application() {
    m_sceneManager.unloadCurrent();
}

int Application::run() {
    VLT_LOG_INFO("Entering main loop.");

    while (!m_window.shouldClose() && !m_sceneManager.exitRequested()) {
        m_window.pollEvents();

        m_time.tick();
        m_sceneManager.update(m_time.deltaSeconds(), m_input);
        m_sceneManager.render();
        m_window.swapBuffers();
        m_input.endFrame();

        m_titleUpdateAccumulator += m_time.deltaSeconds();
        if (m_titleUpdateAccumulator >= 0.5) {
            m_titleUpdateAccumulator = 0.0;
            updateWindowTitle();
        }
    }

    VLT_LOG_INFO("Leaving main loop; unloading scene.");
    m_sceneManager.unloadCurrent();
    return 0;
}

void Application::onFramebufferResize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    glViewport(0, 0, width, height);
}

void Application::updateWindowTitle() {
    char title[160];
    std::snprintf(title, sizeof(title), "%s | %.0f FPS", m_baseTitle.c_str(),
                  m_time.fps());
    m_window.setTitle(title);
}

}  // namespace vlt
