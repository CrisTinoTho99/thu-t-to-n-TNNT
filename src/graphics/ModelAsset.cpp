#include "graphics/ModelAsset.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace vlt {
namespace {

glm::mat4 toGlm(const aiMatrix4x4& matrix) {
    glm::mat4 result(1.0f);
    result[0][0] = matrix.a1;
    result[1][0] = matrix.a2;
    result[2][0] = matrix.a3;
    result[3][0] = matrix.a4;
    result[0][1] = matrix.b1;
    result[1][1] = matrix.b2;
    result[2][1] = matrix.b3;
    result[3][1] = matrix.b4;
    result[0][2] = matrix.c1;
    result[1][2] = matrix.c2;
    result[2][2] = matrix.c3;
    result[3][2] = matrix.c4;
    result[0][3] = matrix.d1;
    result[1][3] = matrix.d2;
    result[2][3] = matrix.d3;
    result[3][3] = matrix.d4;
    return result;
}

void importNode(const aiScene& scene, const aiNode& node,
                const glm::mat4& parentTransform, ImportedModel& result) {
    const glm::mat4 nodeTransform = parentTransform * toGlm(node.mTransformation);
    const std::string nodeName = node.mName.C_Str();
    ++result.nodeNameCounts[nodeName];

    for (unsigned int index = 0; index < node.mNumMeshes; ++index) {
        const aiMesh& source = *scene.mMeshes[node.mMeshes[index]];
        if (source.mNumVertices == 0 || source.mNumFaces == 0) {
            throw std::runtime_error("mesh in node '" + nodeName +
                                     "' has no geometry");
        }

        ImportedMesh mesh;
        mesh.nodeName = nodeName;
        mesh.nodeTransform = nodeTransform;
        mesh.vertices.reserve(static_cast<std::size_t>(source.mNumVertices) * 6);
        for (unsigned int vertexIndex = 0; vertexIndex < source.mNumVertices;
             ++vertexIndex) {
            const aiVector3D& position = source.mVertices[vertexIndex];
            const aiColor4D color =
                source.HasVertexColors(0)
                    ? source.mColors[0][vertexIndex]
                    : aiColor4D(0.72f, 0.48f, 0.22f, 1.0f);
            mesh.vertices.insert(mesh.vertices.end(),
                                 {position.x, position.y, position.z, color.r,
                                  color.g, color.b});
        }

        mesh.indices.reserve(static_cast<std::size_t>(source.mNumFaces) * 3);
        for (unsigned int faceIndex = 0; faceIndex < source.mNumFaces;
             ++faceIndex) {
            const aiFace& face = source.mFaces[faceIndex];
            if (face.mNumIndices != 3) {
                throw std::runtime_error("mesh in node '" + nodeName +
                                         "' contains a non-triangle face");
            }
            for (unsigned int index = 0; index < face.mNumIndices; ++index) {
                mesh.indices.push_back(face.mIndices[index]);
            }
        }
        result.meshes.push_back(std::move(mesh));
    }

    for (unsigned int childIndex = 0; childIndex < node.mNumChildren;
         ++childIndex) {
        importNode(scene, *node.mChildren[childIndex], nodeTransform, result);
    }
}

}  // namespace

bool ImportedModel::hasUniqueNode(const std::string& name) const {
    const auto found = nodeNameCounts.find(name);
    return found != nodeNameCounts.end() && found->second == 1;
}

ImportedModel loadModelAsset(const std::filesystem::path& path) {
    if (!std::filesystem::is_regular_file(path)) {
        throw std::runtime_error("model file does not exist: '" + path.string() +
                                 "'");
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                           aiProcess_ValidateDataStructure);
    if (scene == nullptr || scene->mRootNode == nullptr ||
        (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0) {
        throw std::runtime_error("could not import model '" + path.string() +
                                 "': " + importer.GetErrorString());
    }

    ImportedModel result;
    importNode(*scene, *scene->mRootNode, glm::mat4(1.0f), result);
    if (result.meshes.empty()) {
        throw std::runtime_error("model contains no renderable meshes: '" +
                                 path.string() + "'");
    }
    return result;
}

}  // namespace vlt
