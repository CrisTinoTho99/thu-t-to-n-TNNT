#include "scenes/MapScene.h"

#include <glm/glm.hpp>

#include <memory>
#include <stdexcept>
#include <utility>

#include "core/Input.h"
#include "core/Log.h"
#include "core/SceneManager.h"
#include "graphics/GL.h"
#include "graphics/Model.h"
#include "graphics/ModelAsset.h"
#include "graphics/ShaderProgram.h"
#include "scenes/MainMenuScene.h"
#include "world/MapLoader.h"

namespace vlt {
namespace {

constexpr const char* kVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

uniform mat4 uMvp;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = uMvp * vec4(aPosition, 1.0);
}
)";

constexpr const char* kFragmentShader = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

}  // namespace

MapScene::MapScene(std::string mapId, std::filesystem::path assetRoot,
                   SceneManager& sceneManager)
    : m_sceneManager(sceneManager),
      m_mapId(std::move(mapId)),
      m_assetRoot(std::move(assetRoot)),
      m_manifestPath(m_assetRoot / "maps" / m_mapId / "map.json") {}

MapScene::~MapScene() = default;

void MapScene::onLoad() {
    VLT_LOG_INFO("Opening map '%s' from '%s'.", m_mapId.c_str(),
                 m_manifestPath.string().c_str());

    m_mapLoaded = false;
    MapLoadOptions options;
    options.verifyAssetFiles = true;
    const MapLoadResult load =
        loadMapManifest(m_manifestPath, m_assetRoot, m_mapId, options);

    if (!load.ok) {
        for (const std::string& error : load.errors) {
            VLT_LOG_ERROR("%s", error.c_str());
        }
        if (load.errors.empty()) {
            VLT_LOG_ERROR("Map '%s': manifest validation failed.",
                          m_mapId.c_str());
        }
        VLT_LOG_ERROR("Map '%s' was not loaded; press Esc to return to menu.",
                      m_mapId.c_str());
        return;
    }

    m_mapData = load.data;
    try {
        const std::filesystem::path modelPath =
            resolveMapAsset(m_assetRoot, m_mapId, m_mapData.model);
        ImportedModel importedModel = loadModelAsset(modelPath);
        for (const MapPart& part : m_mapData.parts) {
            if (!importedModel.hasUniqueNode(part.node)) {
                throw std::runtime_error(
                    "part '" + part.id +
                    "' references a missing or ambiguous model node '" +
                    part.node + "'");
            }
        }

        m_camera.setPosition(glm::vec3(m_mapData.startCamera.position.x,
                                       m_mapData.startCamera.position.y,
                                       m_mapData.startCamera.position.z));
        m_camera.setYawPitch(m_mapData.startCamera.yawDegrees,
                             m_mapData.startCamera.pitchDegrees);

        m_shader =
            std::make_unique<ShaderProgram>(kVertexShader, kFragmentShader);
        m_model = std::make_unique<Model>(std::move(importedModel));
        m_mapLoaded = true;

        VLT_LOG_INFO(
            "Loaded map '%s' (schema %d): %s / %s, %s — %zu viewpoints, "
            "%zu parts, %zu hotspots; imported model '%s'.",
            m_mapData.id.c_str(), m_mapData.schemaVersion,
            m_mapData.metadata.nameVi.c_str(), m_mapData.metadata.nameEn.c_str(),
            m_mapData.metadata.location.c_str(), m_mapData.viewpoints.size(),
            m_mapData.parts.size(), m_mapData.hotspots.size(),
            m_mapData.model.c_str());
        glEnable(GL_DEPTH_TEST);
        VLT_LOG_INFO(
            "Controls: WASD/QE move, Shift speeds up, right mouse looks, Esc "
            "returns to the menu.");
    } catch (const std::exception& error) {
        m_model.reset();
        m_shader.reset();
        VLT_LOG_ERROR("Map '%s' model load failed: %s", m_mapId.c_str(),
                      error.what());
    }
}

void MapScene::onUnload() {
    VLT_LOG_INFO("Closing map '%s'.", m_mapId.c_str());
    m_model.reset();
    m_shader.reset();
    m_mapLoaded = false;
}

void MapScene::update(double deltaSeconds, const Input& input) {
    if (input.wasKeyPressed(GLFW_KEY_ESCAPE)) {
        m_sceneManager.requestScene(
            std::make_unique<MainMenuScene>(m_sceneManager, m_assetRoot));
        return;
    }

    if (m_mapLoaded) {
        m_camera.update(input, deltaSeconds);
    }
}

void MapScene::render() {
    int viewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] > 0 && viewport[3] > 0) {
        m_camera.setAspectRatio(static_cast<float>(viewport[2]) /
                                static_cast<float>(viewport[3]));
    }

    if (!m_mapLoaded) {
        glClearColor(0.35f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    glClearColor(0.52f, 0.62f, 0.74f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_shader && m_model) {
        glEnable(GL_DEPTH_TEST);
        const glm::mat4 viewProjection =
            m_camera.projectionMatrix() * m_camera.viewMatrix();
        m_model->draw(*m_shader, viewProjection);
    }
}

}  // namespace vlt
