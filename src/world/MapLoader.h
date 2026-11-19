#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "world/MapData.h"

namespace vlt {

struct MapLoadOptions {
    // Validate that referenced model files exist when the caller is ready to
    // load package assets.
    bool verifyAssetFiles = false;
};

struct MapLoadResult {
    bool ok = false;
    MapData data;
    std::vector<std::string> errors;
    std::filesystem::path manifestPath;
};

// Loads and validates assets/maps/<id>/map.json against schema version 1.
// Every error is prefixed with the map id and the JSON pointer of the failing
// field; the caller must not use `data` unless `ok` is true.
MapLoadResult loadMapManifest(const std::filesystem::path& manifestPath,
                              const std::filesystem::path& assetRoot,
                              const std::string& expectedMapId,
                              const MapLoadOptions& options = {});

// Resolves a manifest reference: relative to the map package, or `common://`
// relative to assets/common/. Returns an empty path when the reference is
// invalid.
std::filesystem::path resolveMapAsset(const std::filesystem::path& assetRoot,
                                      const std::string& mapId,
                                      const std::string& reference);

}  // namespace vlt
