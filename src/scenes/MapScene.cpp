#include "scenes/MapScene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include <imgui.h>

#include "core/Input.h"
#include "core/Log.h"
#include "core/SceneManager.h"
#include "graphics/GL.h"
#include "graphics/HotspotRenderer.h"
#include "graphics/Model.h"
#include "graphics/ModelAsset.h"
#include "graphics/ShaderProgram.h"
#include "scenes/MainMenuScene.h"
#include "ui/ImGuiLayer.h"
#include "world/MapLoader.h"
#include "world/Picking.h"

namespace vlt {
namespace {

constexpr const char* kVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

uniform mat4 uMvp;
uniform mat3 uNormalMatrix;

out vec3 vNormal;
out vec3 vColor;

void main() {
    vNormal = normalize(uNormalMatrix * aNormal);
    vColor = aColor;
    gl_Position = uMvp * vec4(aPosition, 1.0);
}
)";

constexpr const char* kFragmentShader = R"(#version 330 core
in vec3 vNormal;
in vec3 vColor;
uniform vec3 uLightDirection;
uniform int uHighlighted;
out vec4 FragColor;

void main() {
    float diffuse = max(dot(normalize(vNormal), normalize(uLightDirection)), 0.0);
    vec3 shaded = vColor * (0.28 + 0.72 * diffuse);
    if (uHighlighted != 0) {
        shaded = mix(shaded, vec3(1.0, 0.78, 0.22), 0.45);
    }
    FragColor = vec4(shaded, 1.0);
}
)";

constexpr float kHotspotPickRadius = 4.0f;

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
    m_selectedPartIndex.reset();
    m_selectedHotspotIndex.reset();
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
        m_hotspotRenderer =
            std::make_unique<HotspotRenderer>(m_mapData.hotspots);
        m_ui = std::make_unique<ImGuiLayer>();
        m_ui->initialize(glfwGetCurrentContext());
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
        m_ui.reset();
        m_hotspotRenderer.reset();
        m_model.reset();
        m_shader.reset();
        VLT_LOG_ERROR("Map '%s' initialization failed: %s", m_mapId.c_str(),
                      error.what());
    }
}

void MapScene::onUnload() {
    VLT_LOG_INFO("Closing map '%s'.", m_mapId.c_str());
    m_ui.reset();
    m_hotspotRenderer.reset();
    m_model.reset();
    m_shader.reset();
    m_mapLoaded = false;
    m_selectedPartIndex.reset();
    m_selectedHotspotIndex.reset();
}

void MapScene::update(double deltaSeconds, const Input& input) {
    if (input.wasKeyPressed(GLFW_KEY_ESCAPE)) {
        m_sceneManager.requestScene(
            std::make_unique<MainMenuScene>(m_sceneManager, m_assetRoot));
        return;
    }

    if (m_mapLoaded) {
        const bool captureMouse =
            m_ui != nullptr && m_ui->wantsMouseCapture();
        m_camera.update(input, deltaSeconds, !captureMouse);
        if (input.wasMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) &&
            !captureMouse) {
            pickFromCursor(input);
        }
    }
}

void MapScene::pickFromCursor(const Input& input) {
    if (m_model == nullptr) {
        return;
    }

    GLFWwindow* window = glfwGetCurrentContext();
    if (window == nullptr) {
        return;
    }
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    int viewportValues[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewportValues);
    if (windowWidth <= 0 || windowHeight <= 0 || viewportValues[2] <= 0 ||
        viewportValues[3] <= 0) {
        return;
    }

    const float x = static_cast<float>(
        input.mouseX() * static_cast<double>(viewportValues[2]) / windowWidth);
    const float y = static_cast<float>(
        viewportValues[3] -
        input.mouseY() * static_cast<double>(viewportValues[3]) / windowHeight);
    const glm::vec4 viewport(
        static_cast<float>(viewportValues[0]),
        static_cast<float>(viewportValues[1]),
        static_cast<float>(viewportValues[2]),
        static_cast<float>(viewportValues[3]));
    const glm::mat4 view = m_camera.viewMatrix();
    const glm::mat4 projection = m_camera.projectionMatrix();
    const glm::vec3 nearPoint =
        glm::unProject(glm::vec3(x, y, 0.0f), view, projection, viewport);
    const glm::vec3 farPoint =
        glm::unProject(glm::vec3(x, y, 1.0f), view, projection, viewport);
    const Ray ray{m_camera.position(), glm::normalize(farPoint - nearPoint)};

    const std::optional<ModelHit> modelHit = m_model->pick(ray);
    float nearestDistance =
        modelHit ? modelHit->distance : std::numeric_limits<float>::max();
    std::optional<std::size_t> hotspotHit;
    for (std::size_t index = 0; index < m_mapData.hotspots.size(); ++index) {
        const MapHotspot& hotspot = m_mapData.hotspots[index];
        const std::optional<float> distance = intersectRaySphere(
            ray,
            glm::vec3(hotspot.position.x, hotspot.position.y,
                      hotspot.position.z),
            kHotspotPickRadius);
        if (distance && *distance < nearestDistance) {
            nearestDistance = *distance;
            hotspotHit = index;
        }
    }

    m_selectedPartIndex.reset();
    m_selectedHotspotIndex = hotspotHit;
    if (hotspotHit) {
        const MapHotspot& hotspot = m_mapData.hotspots[*hotspotHit];
        VLT_LOG_INFO("Selected hotspot '%s': %s", hotspot.id.c_str(),
                     hotspot.nameVi.c_str());
        return;
    }

    if (modelHit) {
        for (std::size_t index = 0; index < m_mapData.parts.size(); ++index) {
            if (m_mapData.parts[index].node == modelHit->nodeName) {
                m_selectedPartIndex = index;
                const MapPart& part = m_mapData.parts[index];
                VLT_LOG_INFO("Selected part '%s': %s", part.id.c_str(),
                             part.nameVi.c_str());
                return;
            }
        }
    }
    VLT_LOG_INFO("Selection cleared.");
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
        const glm::vec3 lightDirection =
            glm::normalize(glm::vec3(-0.4f, 1.0f, 0.35f));
        const std::string highlightedNode =
            m_selectedPartIndex
                ? m_mapData.parts[*m_selectedPartIndex].node
                : std::string();
        m_model->draw(*m_shader, viewProjection, lightDirection,
                      highlightedNode);
        if (m_hotspotRenderer) {
            m_hotspotRenderer->draw(
                viewProjection,
                m_selectedHotspotIndex.value_or(
                    std::numeric_limits<std::size_t>::max()));
        }
    }
}

void MapScene::renderUI() {
    if (!m_mapLoaded || m_ui == nullptr) {
        return;
    }

    m_ui->newFrame();
    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360.0f, 480.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Map information");
    ImGui::Text("%s", m_mapData.metadata.nameVi.c_str());
    ImGui::TextDisabled("%s — %s", m_mapData.metadata.nameEn.c_str(),
                        m_mapData.metadata.location.c_str());
    ImGui::Separator();
    ImGui::TextWrapped("Click a model part or hotspot to inspect it.");
    ImGui::Spacing();

    if (m_selectedPartIndex) {
        const MapPart& part = m_mapData.parts[*m_selectedPartIndex];
        ImGui::Text("Selected part: %s", part.nameVi.c_str());
        if (!part.nameEn.empty()) {
            ImGui::TextDisabled("%s", part.nameEn.c_str());
        }
        ImGui::TextWrapped("%s", part.description.c_str());
        ImGui::Separator();
    } else if (m_selectedHotspotIndex) {
        const MapHotspot& hotspot =
            m_mapData.hotspots[*m_selectedHotspotIndex];
        ImGui::Text("Selected hotspot: %s", hotspot.nameVi.c_str());
        ImGui::TextWrapped("%s", hotspot.description.c_str());
        ImGui::Separator();
    }

    if (ImGui::CollapsingHeader("Parts", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (std::size_t index = 0; index < m_mapData.parts.size(); ++index) {
            const MapPart& part = m_mapData.parts[index];
            ImGui::PushID(part.id.c_str());
            if (ImGui::Selectable(
                    part.nameVi.c_str(),
                    m_selectedPartIndex && *m_selectedPartIndex == index)) {
                m_selectedPartIndex = index;
                m_selectedHotspotIndex.reset();
            }
            ImGui::PopID();
        }
    }

    if (!m_mapData.hotspots.empty() &&
        ImGui::CollapsingHeader("Hotspots", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (std::size_t index = 0; index < m_mapData.hotspots.size(); ++index) {
            const MapHotspot& hotspot = m_mapData.hotspots[index];
            ImGui::PushID(hotspot.id.c_str());
            if (ImGui::Selectable(
                    hotspot.nameVi.c_str(),
                    m_selectedHotspotIndex &&
                        *m_selectedHotspotIndex == index)) {
                m_selectedHotspotIndex = index;
                m_selectedPartIndex.reset();
            }
            ImGui::PopID();
        }
    }
    ImGui::End();
    m_ui->render();
}

}  // namespace vlt
