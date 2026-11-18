#include "core/SceneManager.h"

#include <exception>
#include <utility>

#include "core/Log.h"
#include "scenes/IScene.h"

namespace vlt {

SceneManager::~SceneManager() {
    unloadCurrent();
}

void SceneManager::requestScene(std::unique_ptr<IScene> nextScene) {
    if (nextScene == nullptr) {
        VLT_LOG_WARN("Ignoring request for a null scene.");
        return;
    }

    if (m_pending != nullptr) {
        VLT_LOG_WARN("Replacing pending scene '%s' with '%s'.",
                     m_pending->name(), nextScene->name());
    }

    m_pending = std::move(nextScene);
}

void SceneManager::update(double deltaSeconds, const Input& input) {
    applyPendingScene();

    if (m_current != nullptr) {
        m_current->update(deltaSeconds, input);
    }
}

void SceneManager::render() {
    if (m_current != nullptr) {
        m_current->render();
        m_current->renderUI();
    }
}

void SceneManager::unloadCurrent() {
    if (m_current == nullptr) {
        return;
    }

    VLT_LOG_INFO("Unloading scene '%s'.", m_current->name());
    try {
        m_current->onUnload();
    } catch (const std::exception& error) {
        // Report rather than hide, but keep shutdown/transition going.
        VLT_LOG_ERROR("Scene '%s' failed during onUnload: %s",
                      m_current->name(), error.what());
    } catch (...) {
        VLT_LOG_ERROR("Scene '%s' failed during onUnload with an unknown error.",
                      m_current->name());
    }
    m_current.reset();
}

void SceneManager::applyPendingScene() {
    if (m_pending == nullptr) {
        return;
    }

    std::unique_ptr<IScene> nextScene = std::move(m_pending);
    unloadCurrent();

    VLT_LOG_INFO("Loading scene '%s'.", nextScene->name());
    nextScene->onLoad();
    m_current = std::move(nextScene);
}

}  // namespace vlt
