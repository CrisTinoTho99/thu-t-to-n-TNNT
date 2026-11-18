#include "scenes/MapScene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <utility>

#include "core/Input.h"
#include "core/Log.h"
#include "core/SceneManager.h"
#include "graphics/GL.h"
#include "graphics/ShaderProgram.h"
#include "graphics/TestCube.h"
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

    const MapLoadResult load =
        loadMapManifest(m_manifestPath, m_assetRoot, m_mapId);

    if (!load.ok) {
        for (const std::string& error : load.errors) {
            VLT_LOG_ERROR("%s", error.c_str());
        }
        VLT_LOG_ERROR(
            "Map '%s' was not loaded. Placeholder scene shows an error "
            "background; press Esc to return to the menu.",
            m_mapId.c_str());
        return;
    }

    m_mapData = load.data;
    m_mapLoaded = true;

    VLT_LOG_INFO(
        "Loaded map '%s' (schema %d): %s / %s, %s — %zu viewpoints, "
        "%zu parts, %zu hotspots.",
        m_mapData.id.c_str(), m_mapData.schemaVersion,
        m_mapData.metadata.nameVi.c_str(), m_mapData.metadata.nameEn.c_str(),
        m_mapData.metadata.location.c_str(), m_mapData.viewpoints.size(),
        m_mapData.parts.size(), m_mapData.hotspots.size());

    m_camera.setPosition(glm::vec3(m_mapData.startCamera.position.x,
                                   m_mapData.startCamera.position.y,
                                   m_mapData.startCamera.position.z));
    m_camera.setYawPitch(m_mapData.startCamera.yawDegrees,
                         m_mapData.startCamera.pitchDegrees);

    VLT_LOG_INFO(
        "Model '%s' is NOT loaded yet (no GLB importer); showing placeholder "
        "test cube instead.",
        m_mapData.model.c_str());

    m_shader = std::make_unique<ShaderProgram>(kVertexShader, kFragmentShader);
    m_cube = std::make_unique<TestCube>();

    glEnable(GL_DEPTH_TEST);
    VLT_LOG_INFO(
        "Controls: WASD/QE move, Shift speeds up, right mouse looks, Esc "
        "returns to the menu.");
}

void MapScene::onUnload() {
    VLT_LOG_INFO("Closing map '%s'.", m_mapId.c_str());
    m_cube.reset();
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

    if (m_shader && m_cube) {
        glEnable(GL_DEPTH_TEST);
        // Placeholder geometry is positioned in front of the manifest start
        // camera so the camera integration is exercised before a GLB exists.
        const glm::mat4 model = glm::translate(
            glm::mat4(1.0f), m_camera.position() + m_camera.forward() * 8.0f);
        const glm::mat4 mvp =
            m_camera.projectionMatrix() * m_camera.viewMatrix() * model;
        m_shader->use();
        m_shader->setMat4("uMvp", mvp);
        m_cube->draw();
    }
}

}  // namespace vlt
