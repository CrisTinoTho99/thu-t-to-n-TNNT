#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "graphics/ModelAsset.h"
#include "world/Picking.h"

namespace vlt {

class ShaderProgram;

struct ModelHit {
    std::string nodeName;
    float distance = 0.0f;
};

// Owns the OpenGL buffers for one imported static model. Construct and destroy
// it while the map's OpenGL context is current.
class Model {
public:
    explicit Model(ImportedModel model);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void draw(const ShaderProgram& shader, const glm::mat4& viewProjection,
              const glm::vec3& lightDirection,
              const std::string& highlightedNode = {}) const;
    std::optional<ModelHit> pick(const Ray& ray) const;

private:
    class Mesh;
    std::vector<std::unique_ptr<Mesh>> m_meshes;
};

}  // namespace vlt
