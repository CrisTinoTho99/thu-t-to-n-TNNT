#pragma once

#include <memory>

namespace vlt {

class IScene;
class Input;

// Owns the active scene. Scene changes are deferred to the next update() so a
// scene can request a transition from inside its own update() without deleting
// itself while it is running.
class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    // Takes ownership. A pending request is replaced and a warning is logged.
    void requestScene(std::unique_ptr<IScene> nextScene);

    // Applies a pending scene change, then updates the active scene.
    void update(double deltaSeconds, const Input& input);
    void render();

    // Unloads and destroys the active scene. Safe to call more than once.
    void unloadCurrent();

    IScene* current() const { return m_current.get(); }

    void requestExit() { m_exitRequested = true; }
    bool exitRequested() const { return m_exitRequested; }

private:
    void applyPendingScene();

    std::unique_ptr<IScene> m_current;
    std::unique_ptr<IScene> m_pending;
    bool m_exitRequested = false;
};

}  // namespace vlt
