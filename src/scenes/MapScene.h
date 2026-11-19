#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "graphics/Camera.h"
#include "scenes/IScene.h"
#include "world/MapData.h"

namespace vlt {

class Input;
class HotspotRenderer;
class ImGuiLayer;
class Model;
class SceneManager;
class ShaderProgram;

// Shared scene for every map package. It loads the package manifest, reports
// schema errors with the map id and JSON pointer, imports the package model,
// and renders it with the shared fly camera.
class MapScene : public IScene {
public:
    MapScene(std::string mapId, std::filesystem::path assetRoot,
             SceneManager& sceneManager);
    ~MapScene() override;

    const char* name() const override { return "MapScene"; }

    void onLoad() override;
    void onUnload() override;
    void update(double deltaSeconds, const Input& input) override;
    void render() override;
    void renderUI() override;

private:
    void pickFromCursor(const Input& input);

    SceneManager& m_sceneManager;
    std::string m_mapId;
    std::filesystem::path m_assetRoot;
    std::filesystem::path m_manifestPath;
    bool m_mapLoaded = false;
    MapData m_mapData;

    std::unique_ptr<ShaderProgram> m_shader;
    std::unique_ptr<Model> m_model;
    std::unique_ptr<HotspotRenderer> m_hotspotRenderer;
    std::unique_ptr<ImGuiLayer> m_ui;
    Camera m_camera;
    std::optional<std::size_t> m_selectedPartIndex;
    std::optional<std::size_t> m_selectedHotspotIndex;
};

}  // namespace vlt
