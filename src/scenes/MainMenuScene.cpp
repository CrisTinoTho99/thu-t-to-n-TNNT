#include "scenes/MainMenuScene.h"

#include <memory>
#include <string>
#include <utility>

#include "core/Input.h"
#include "core/Log.h"
#include "core/SceneManager.h"
#include "graphics/GL.h"
#include "scenes/MapScene.h"

namespace vlt {

MainMenuScene::MainMenuScene(SceneManager& sceneManager,
                             std::filesystem::path assetRoot)
    : m_sceneManager(sceneManager), m_assetRoot(std::move(assetRoot)) {}

void MainMenuScene::onLoad() {
    VLT_LOG_INFO("Main menu (placeholder): press 1 for Eiffel, Esc to quit.");
    if (m_assetRoot.empty()) {
        VLT_LOG_WARN(
            "Asset root not found; map manifests will fail to load. Run the "
            "executable from inside the project or its build directory.");
    }
}

void MainMenuScene::onUnload() {}

void MainMenuScene::update(double /*deltaSeconds*/, const Input& input) {
    if (input.wasKeyPressed(GLFW_KEY_ESCAPE)) {
        VLT_LOG_INFO("Exit requested from main menu.");
        m_sceneManager.requestExit();
        return;
    }

    if (input.wasKeyPressed(GLFW_KEY_1)) {
        openMap("eiffel");
    } else if (input.wasKeyPressed(GLFW_KEY_2)) {
        VLT_LOG_WARN("Map 'liberty' is not available yet (no package created).");
    } else if (input.wasKeyPressed(GLFW_KEY_3)) {
        VLT_LOG_WARN("Map 'bigben' is not available yet (no package created).");
    }
}

void MainMenuScene::render() {
    glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MainMenuScene::openMap(const char* mapId) {
    if (m_assetRoot.empty()) {
        VLT_LOG_ERROR("Cannot open map '%s': asset root is unknown.", mapId);
        return;
    }

    m_sceneManager.requestScene(
        std::make_unique<MapScene>(mapId, m_assetRoot, m_sceneManager));
}

}  // namespace vlt
