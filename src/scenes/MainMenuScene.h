#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "scenes/IScene.h"

namespace vlt {

class ImGuiLayer;
class Input;
class SceneManager;

class MainMenuScene : public IScene {
public:
    MainMenuScene(SceneManager& sceneManager,
                  std::filesystem::path assetRoot);
    ~MainMenuScene() override;

    const char* name() const override { return "MainMenuScene"; }

    void onLoad() override;
    void onUnload() override;
    void update(double deltaSeconds, const Input& input) override;
    void render() override;
    void renderUI() override;

private:
    struct MapEntry {
        std::string id;
        std::string nameVi;
        std::string location;
        std::string status;
        int shortcut = 0;
        bool available = false;
    };

    void discoverMaps();
    void openMap(const MapEntry& map);

    SceneManager& m_sceneManager;
    std::filesystem::path m_assetRoot;
    std::vector<MapEntry> m_maps;
    std::unique_ptr<ImGuiLayer> m_ui;
};

}  // namespace vlt
