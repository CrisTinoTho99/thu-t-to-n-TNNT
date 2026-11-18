#pragma once

#include <filesystem>

namespace vlt::paths {

// Locates the "assets" directory by walking up from the working directory and
// from the executable directory (max. 6 levels). Returns an empty path when no
// assets/maps directory is found; callers must report that instead of silently
// using a wrong path.
std::filesystem::path findAssetRoot(const std::filesystem::path& executablePath);

}  // namespace vlt::paths
