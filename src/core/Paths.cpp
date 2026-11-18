#include "core/Paths.h"

#include <vector>

namespace vlt::paths {
namespace {

bool containsAssets(const std::filesystem::path& dir) {
    std::error_code error;
    return std::filesystem::is_directory(dir / "assets" / "maps", error);
}

}  // namespace

std::filesystem::path findAssetRoot(const std::filesystem::path& executablePath) {
    std::vector<std::filesystem::path> startDirectories;
    std::error_code error;

    const std::filesystem::path workingDirectory =
        std::filesystem::current_path(error);
    if (!error) {
        startDirectories.push_back(workingDirectory);
    }

    if (!executablePath.empty()) {
        const std::filesystem::path absoluteExecutable =
            std::filesystem::absolute(executablePath, error);
        if (!error) {
            startDirectories.push_back(absoluteExecutable.parent_path());
        }
    }

    for (const std::filesystem::path& start : startDirectories) {
        std::filesystem::path directory = start;
        for (int depth = 0; depth < 6; ++depth) {
            if (containsAssets(directory)) {
                return directory / "assets";
            }
            const std::filesystem::path parent = directory.parent_path();
            if (parent.empty() || parent == directory) {
                break;
            }
            directory = parent;
        }
    }

    return {};
}

}  // namespace vlt::paths
