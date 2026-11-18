#pragma once

#include <filesystem>
#include <string>

#include "core/Input.h"
#include "core/SceneManager.h"
#include "core/Time.h"
#include "core/Window.h"

namespace vlt {

// Owns the window, input, timing and scene manager, and runs the main loop.
class Application {
public:
    Application(int argc, char** argv);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Returns the process exit code: 0 on a clean shutdown, 1 on fatal errors.
    int run();

private:
    void onFramebufferResize(int width, int height);
    void updateWindowTitle();

    Input m_input;
    Window m_window;
    Time m_time;
    SceneManager m_sceneManager;

    std::filesystem::path m_assetRoot;
    std::string m_baseTitle;
    double m_titleUpdateAccumulator = 0.0;
};

}  // namespace vlt
