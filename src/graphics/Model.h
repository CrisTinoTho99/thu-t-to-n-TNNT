#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

#include "graphics/ModelAsset.h"

namespace vlt {

class ShaderProgram;

// Owns the OpenGL buffers for one imported static model. Construct and destroy
// it while the map's OpenGL context is current.
class Model {
public:
    explicit Model(ImportedModel model);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void draw(const ShaderProgram& shader,
              const glm::mat4& viewProjection) const;

private:
    class Mesh;
    std::vector<std::unique_ptr<Mesh>> m_meshes;
};

}  // namespace vlt
