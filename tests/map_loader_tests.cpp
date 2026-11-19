#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "world/MapLoader.h"

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& label,
           const std::vector<std::string>& errors = {}) {
    ++g_checks;
    if (condition) {
        std::cout << "[PASS] " << label << '\n';
        return;
    }
    ++g_failures;
    std::cout << "[FAIL] " << label << '\n';
    for (const std::string& error : errors) {
        std::cout << "       " << error << '\n';
    }
}

void checkFails(vlt::MapLoadResult& result, const std::string& label,
                const std::string& expectedFragment) {
    bool found = false;
    for (const std::string& error : result.errors) {
        if (error.find(expectedFragment) != std::string::npos) {
            found = true;
            break;
        }
    }
    check(!result.ok && found,
          label + " (expected error containing '" + expectedFragment + "')",
          result.errors);
}

std::string validManifest() {
    return R"({
  "schema_version": 1,
  "id": "test",
  "metadata": {
    "name_vi": "Test",
    "name_en": "Test",
    "location": "Nowhere"
  },
  "model": "models/test.glb",
  "skybox": {
    "day": "common://skyboxes/day",
    "dusk": "common://skyboxes/dusk",
    "night": "common://skyboxes/night"
  },
  "start_camera": {
    "position": [0.0, 1.7, 5.0],
    "yaw": 0.0,
    "pitch": 10.0
  },
  "viewpoints": [
    { "key": 1, "name_vi": "One", "position": [0, 1.7, 5], "target": [0, 1.7, 0] },
    { "key": 2, "name_vi": "Two", "position": [2, 1.7, 5], "target": [0, 1.7, 0] }
  ],
  "tour_path": [ [0, 1, 0], [1, 1, 0], [2, 1, 0] ],
  "parts": [
    { "id": "P01", "node": "Node01", "name_vi": "Part", "description": "Desc" }
  ],
  "hotspots": [
    { "id": "spot_one", "name_vi": "Spot", "position": [0, 1, 0], "description": "Desc" }
  ]
})";
}

std::string replaceAll(std::string text, const std::string& from,
                       const std::string& to) {
    std::size_t position = 0;
    while ((position = text.find(from, position)) != std::string::npos) {
        text.replace(position, from.size(), to);
        position += to.size();
    }
    return text;
}

std::filesystem::path g_root;

std::filesystem::path writeCase(const std::string& name,
                                const std::string& content) {
    const std::filesystem::path directory = g_root / name;
    std::filesystem::create_directories(directory);
    const std::filesystem::path file = directory / "map.json";
    std::ofstream stream(file, std::ios::binary | std::ios::trunc);
    stream << content;
    stream.close();
    return file;
}

std::filesystem::path g_assetRoot;

vlt::MapLoadResult load(const std::filesystem::path& manifest,
                       const std::string& mapId = "test") {
    return vlt::loadMapManifest(manifest, g_assetRoot, mapId);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: vlt_map_tests <assets-directory>\n";
        return 2;
    }
    g_assetRoot = std::filesystem::path(argv[1]);
    g_root = std::filesystem::temp_directory_path() / "vlt_map_tests";
    std::error_code error;
    std::filesystem::remove_all(g_root, error);
    std::filesystem::create_directories(g_root);

    // 1. The real Eiffel blockout manifest is valid.
    {
        vlt::MapLoadResult result = vlt::loadMapManifest(
            g_assetRoot / "maps" / "eiffel" / "map.json", g_assetRoot,
            "eiffel");
        check(result.ok, "eiffel blockout manifest is valid", result.errors);
        check(result.data.metadata.nameEn == "Eiffel Tower",
              "eiffel metadata parsed", result.errors);
        check(result.data.viewpoints.size() == 2, "eiffel viewpoints parsed",
              result.errors);
        check(!result.data.model.empty(), "eiffel model reference parsed",
              result.errors);
    }

    // 2. Valid synthetic manifest.
    {
        const auto manifest = writeCase("valid", validManifest());
        vlt::MapLoadResult result = load(manifest);
        check(result.ok, "valid synthetic manifest", result.errors);
        check(result.data.hasSkybox, "valid skybox parsed", result.errors);
        check(result.data.tourPath.size() == 3, "valid tour path parsed",
              result.errors);
        check(result.data.startCamera.pitchDegrees == 10.0f,
              "camera pitch parsed", result.errors);
    }

    // 3. Unknown root field.
    {
        auto body = replaceAll(validManifest(), "\"id\": \"test\",",
                               "\"id\": \"test\", \"unexpected\": 1,");
        const auto manifest = writeCase("unknown_field", body);
        auto result = load(manifest);
        checkFails(result, "unknown root field rejected", "/unexpected");
    }

    // 4. Unsupported schema version.
    {
        auto body = replaceAll(validManifest(), "\"schema_version\": 1",
                               "\"schema_version\": 2");
        const auto manifest = writeCase("schema_version", body);
        auto result = load(manifest);
        checkFails(result, "unsupported schema version rejected",
                   "unsupported schema_version");
    }

    // 5. Duplicate viewpoint key.
    {
        auto body = replaceAll(validManifest(), "\"key\": 2", "\"key\": 1");
        const auto manifest = writeCase("duplicate_key", body);
        auto result = load(manifest);
        checkFails(result, "duplicate viewpoint key rejected",
                   "duplicate viewpoint key");
    }

    // 6. Invalid asset path.
    {
        auto body = replaceAll(validManifest(), "\"models/test.glb\"",
                               "\"../escape.glb\"");
        const auto manifest = writeCase("bad_path", body);
        auto result = load(manifest);
        checkFails(result, "parent-directory asset path rejected", "/model");
    }

    // 7. Map id must match the package directory.
    {
        auto body = replaceAll(validManifest(), "\"id\": \"test\"",
                               "\"id\": \"other\"");
        const auto manifest = writeCase("id_mismatch", body);
        auto result = load(manifest);
        checkFails(result, "id/directory mismatch rejected", "/id");
    }

    // 8. Invalid part id and duplicate hotspot/part id.
    {
        auto body = replaceAll(validManifest(), "\"id\": \"P01\"",
                               "\"id\": \"1bad\"");
        const auto manifest = writeCase("bad_part_id", body);
        auto result = load(manifest);
        checkFails(result, "invalid part id rejected", "/parts/0/id");
    }
    {
        auto body = replaceAll(validManifest(), "\"id\": \"spot_one\"",
                               "\"id\": \"P01\"");
        const auto manifest = writeCase("duplicate_entity_id", body);
        auto result = load(manifest);
        checkFails(result, "hotspot/part duplicate id rejected", "duplicate id");
    }

    // 9. Missing metadata field.
    {
        auto body = replaceAll(validManifest(), "\"name_vi\": \"Test\",",
                               "");
        const auto manifest = writeCase("missing_metadata", body);
        auto result = load(manifest);
        checkFails(result, "missing metadata name_vi rejected",
                   "/metadata/name_vi");
    }

    // 10. Viewpoint count and consecutive duplicate tour points.
    {
        auto body = replaceAll(
            validManifest(),
            R"("viewpoints": [
    { "key": 1, "name_vi": "One", "position": [0, 1.7, 5], "target": [0, 1.7, 0] },
    { "key": 2, "name_vi": "Two", "position": [2, 1.7, 5], "target": [0, 1.7, 0] }
  ])",
            R"("viewpoints": [])");
        const auto manifest = writeCase("empty_viewpoints", body);
        auto result = load(manifest);
        checkFails(result, "empty viewpoints rejected", "between 1 and 9");
    }
    {
        auto body = replaceAll(validManifest(),
                               "[ [0, 1, 0], [1, 1, 0], [2, 1, 0] ]",
                               "[ [0, 1, 0], [0, 1, 0] ]");
        const auto manifest = writeCase("duplicate_tour", body);
        auto result = load(manifest);
        checkFails(result, "duplicate consecutive tour points rejected",
                   "consecutive tour points");
    }

    // 11. Invalid common:// path and bad JSON syntax.
    {
        auto body = replaceAll(validManifest(), "common://skyboxes/day",
                               "common://../day");
        const auto manifest = writeCase("bad_common", body);
        auto result = load(manifest);
        checkFails(result, "common:// traversal rejected", "/skybox/day");
    }
    {
        const auto manifest = writeCase("bad_json", "{ not json");
        auto result = load(manifest);
        checkFails(result, "malformed JSON rejected", "JSON parse error");
    }

    // 12. Missing manifest.
    {
        auto result = load(g_root / "does_not_exist" / "map.json");
        checkFails(result, "missing manifest rejected", "manifest not found");
    }

    std::filesystem::remove_all(g_root, error);
    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks
              << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
