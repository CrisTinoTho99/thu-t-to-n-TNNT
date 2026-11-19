#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "graphics/ModelAsset.h"
#include "world/MapLoader.h"

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& label) {
    ++g_checks;
    if (condition) {
        std::cout << "[PASS] " << label << '\n';
        return;
    }
    ++g_failures;
    std::cout << "[FAIL] " << label << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: vlt_model_tests <assets-root>\n";
        return 2;
    }

    const std::filesystem::path assetRoot = argv[1];
    const std::filesystem::path manifestPath =
        assetRoot / "maps" / "eiffel" / "map.json";
    vlt::MapLoadOptions options;
    options.verifyAssetFiles = true;
    const vlt::MapLoadResult manifest =
        vlt::loadMapManifest(manifestPath, assetRoot, "eiffel", options);
    check(manifest.ok, "Eiffel manifest references an existing model");
    if (!manifest.ok) {
        for (const std::string& error : manifest.errors) {
            std::cerr << error << '\n';
        }
        return 1;
    }

    const std::filesystem::path modelPath =
        vlt::resolveMapAsset(assetRoot, manifest.data.id, manifest.data.model);
    try {
        const vlt::ImportedModel model = vlt::loadModelAsset(modelPath);
        check(!model.meshes.empty(), "GLB contains renderable geometry");
        check(model.hasUniqueNode("E01_Pillars"), "pillar node is uniquely named");
        check(model.hasUniqueNode("E02_CrossBracing"),
              "cross-bracing node is uniquely named");
        check(model.hasUniqueNode("E03_Platforms"),
              "platform node is uniquely named");
        check(model.hasUniqueNode("E07_Antenna"),
              "antenna node is uniquely named");
        check(!model.hasUniqueNode("missing_node"),
              "missing node is not reported as present");

        bool allPartsMatch = true;
        for (const vlt::MapPart& part : manifest.data.parts) {
            allPartsMatch =
                allPartsMatch && model.hasUniqueNode(part.node);
        }
        check(allPartsMatch, "every manifest part resolves to one GLB node");

        bool missingFileRejected = false;
        try {
            vlt::loadModelAsset(modelPath.parent_path() / "missing.glb");
        } catch (const std::runtime_error&) {
            missingFileRejected = true;
        }
        check(missingFileRejected, "missing GLB produces an explicit error");
    } catch (const std::exception& error) {
        std::cerr << "Model import failed: " << error.what() << '\n';
        check(false, "GLB imports without an exception");
    }

    std::cout << g_checks - g_failures << "/" << g_checks
              << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
