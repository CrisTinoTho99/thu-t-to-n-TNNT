#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "world/MapData.h"

namespace vlt {

struct MapLoadOptions {
    // The manifest milestone validates paths syntactically only, because the
    // Eiffel package has no GLB yet. Asset-resolution code can enable this to
    // require the referenced files to exist.
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
// invalid. Callers normally do not need this before the asset pipeline exists.
std::filesystem::path resolveMapAsset(const std::filesystem::path& assetRoot,
                                      const std::string& mapId,
                                      const std::string& reference);

}  // namespace vlt
