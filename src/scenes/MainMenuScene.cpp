#include "scenes/MainMenuScene.h"

#include <array>
#include <cstddef>
#include <memory>
#include <system_error>
#include <utility>

#include <imgui.h>

#include "core/Input.h"
#include "core/Log.h"
#include "core/SceneManager.h"
#include "graphics/GL.h"
#include "scenes/MapScene.h"
#include "ui/ImGuiLayer.h"
#include "world/MapLoader.h"

namespace vlt {
namespace {

struct MapDefinition {
    const char* id;
    const char* nameVi;
    const char* location;
    int shortcut;
};

constexpr std::array<MapDefinition, 3> kMapDefinitions = {{
    {"eiffel", "Tháp Eiffel", "Paris, Pháp", GLFW_KEY_1},
    {"liberty", "Tượng Nữ thần Tự do", "New York, Mỹ", GLFW_KEY_2},
    {"bigben", "Tháp Elizabeth (Big Ben)", "London, Anh", GLFW_KEY_3},
}};

}  // namespace

MainMenuScene::MainMenuScene(SceneManager& sceneManager,
                             std::filesystem::path assetRoot)
    : m_sceneManager(sceneManager), m_assetRoot(std::move(assetRoot)) {}

MainMenuScene::~MainMenuScene() = default;

void MainMenuScene::onLoad() {
    m_ui = std::make_unique<ImGuiLayer>();
    m_ui->initialize(glfwGetCurrentContext());
    discoverMaps();

    std::size_t availableCount = 0;
    for (const MapEntry& map : m_maps) {
        if (map.available) {
            ++availableCount;
        }
    }
    VLT_LOG_INFO("Main menu loaded: %zu of %zu map packages are ready to open.",
                 availableCount, m_maps.size());
    VLT_LOG_INFO("Select a map with its button or shortcut; press Esc to quit.");
}

void MainMenuScene::onUnload() {
    m_ui.reset();
    m_maps.clear();
}

void MainMenuScene::discoverMaps() {
    m_maps.clear();
    for (const MapDefinition& definition : kMapDefinitions) {
        MapEntry entry;
        entry.id = definition.id;
        entry.nameVi = definition.nameVi;
        entry.location = definition.location;
        entry.shortcut = definition.shortcut;

        if (m_assetRoot.empty()) {
            entry.status = "Không tìm thấy thư mục assets.";
            m_maps.push_back(std::move(entry));
            continue;
        }

        const std::filesystem::path manifestPath =
            m_assetRoot / "maps" / entry.id / "map.json";
        std::error_code fileError;
        const bool manifestExists =
            std::filesystem::is_regular_file(manifestPath, fileError);
        if (fileError == std::errc::no_such_file_or_directory ||
            fileError == std::errc::not_a_directory) {
            entry.status = "Chưa có gói map; cần map.json và model GLB.";
            m_maps.push_back(std::move(entry));
            continue;
        }
        if (fileError) {
            entry.status = "Không thể kiểm tra manifest.";
            VLT_LOG_WARN("Cannot inspect map '%s' manifest '%s': %s",
                         entry.id.c_str(), manifestPath.string().c_str(),
                         fileError.message().c_str());
            m_maps.push_back(std::move(entry));
            continue;
        }
        if (!manifestExists) {
            entry.status = "Chưa có gói map; cần map.json và model GLB.";
            m_maps.push_back(std::move(entry));
            continue;
        }

        MapLoadOptions options;
        options.verifyAssetFiles = true;
        const MapLoadResult result = loadMapManifest(
            manifestPath, m_assetRoot, entry.id, options);
        if (!result.ok) {
            entry.status = "Gói map chưa hợp lệ; xem log để biết chi tiết.";
            for (const std::string& error : result.errors) {
                VLT_LOG_WARN("%s", error.c_str());
            }
            m_maps.push_back(std::move(entry));
            continue;
        }

        entry.nameVi = result.data.metadata.nameVi;
        entry.location = result.data.metadata.location;
        entry.status = "Sẵn sàng";
        entry.available = true;
        m_maps.push_back(std::move(entry));
    }
}

void MainMenuScene::update(double /*deltaSeconds*/, const Input& input) {
    if (input.wasKeyPressed(GLFW_KEY_ESCAPE)) {
        VLT_LOG_INFO("Exit requested from main menu.");
        m_sceneManager.requestExit();
        return;
    }

    for (const MapEntry& map : m_maps) {
        if (input.wasKeyPressed(map.shortcut)) {
            openMap(map);
            return;
        }
    }
}

void MainMenuScene::render() {
    glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MainMenuScene::renderUI() {
    if (m_ui == nullptr) {
        return;
    }

    m_ui->newFrame();
    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(460.0f, 430.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Virtual Landmark Tour");
    ImGui::TextWrapped(
        "Chọn map có manifest hợp lệ và tệp model tồn tại. GLB và node được "
        "kiểm tra khi mở map.");
    ImGui::Separator();

    for (const MapEntry& map : m_maps) {
        ImGui::PushID(map.id.c_str());
        if (map.available) {
            if (ImGui::Button(map.nameVi.c_str(), ImVec2(-1.0f, 0.0f))) {
                openMap(map);
            }
            ImGui::TextDisabled("%s  |  Phim %d", map.location.c_str(),
                                map.shortcut - GLFW_KEY_0);
        } else {
            ImGui::BeginDisabled();
            ImGui::Button(map.nameVi.c_str(), ImVec2(-1.0f, 0.0f));
            ImGui::EndDisabled();
            ImGui::TextDisabled("%s", map.status.c_str());
        }
        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::Separator();
    ImGui::TextDisabled("Esc để thoát.");
    ImGui::End();
    m_ui->render();
}

void MainMenuScene::openMap(const MapEntry& map) {
    if (!map.available) {
        VLT_LOG_WARN("Map '%s' is not ready: %s", map.id.c_str(),
                     map.status.c_str());
        return;
    }
    m_sceneManager.requestScene(
        std::make_unique<MapScene>(map.id, m_assetRoot, m_sceneManager));
}

}  // namespace vlt
