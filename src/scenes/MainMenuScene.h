#pragma once

#include <filesystem>

#include "scenes/IScene.h"

namespace vlt {

class Input;
class SceneManager;

// Placeholder menu: no ImGui cards yet, only keyboard shortcuts and a solid
// background. Map 1 opens the Eiffel blockout; maps 2 and 3 are not available.
class MainMenuScene : public IScene {
public:
    MainMenuScene(SceneManager& sceneManager,
                  std::filesystem::path assetRoot);

    const char* name() const override { return "MainMenuScene"; }

    void onLoad() override;
    void onUnload() override;
    void update(double deltaSeconds, const Input& input) override;
    void render() override;

private:
    void openMap(const char* mapId);

    SceneManager& m_sceneManager;
    std::filesystem::path m_assetRoot;
};

}  // namespace vlt
