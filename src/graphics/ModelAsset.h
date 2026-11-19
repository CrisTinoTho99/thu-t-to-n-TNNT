#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace vlt {

struct ImportedMesh {
    std::string nodeName;
    glm::mat4 nodeTransform{1.0f};
    std::vector<float> vertices;
    std::vector<std::uint32_t> indices;
};

struct ImportedModel {
    std::vector<ImportedMesh> meshes;
    std::unordered_map<std::string, std::size_t> nodeNameCounts;

    bool hasUniqueNode(const std::string& name) const;
};

// Imports a static model on the CPU. Throws std::runtime_error with the source
// path and importer diagnostic if the file is absent, invalid, or has no mesh.
ImportedModel loadModelAsset(const std::filesystem::path& path);

}  // namespace vlt
