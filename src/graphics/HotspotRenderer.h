#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <memory>
#include <vector>

#include "world/MapData.h"

namespace vlt {

class ShaderProgram;

class HotspotRenderer {
public:
    explicit HotspotRenderer(const std::vector<MapHotspot>& hotspots);
    ~HotspotRenderer();

    HotspotRenderer(const HotspotRenderer&) = delete;
    HotspotRenderer& operator=(const HotspotRenderer&) = delete;

    void draw(const glm::mat4& viewProjection,
              std::size_t selectedIndex) const;

private:
    std::unique_ptr<ShaderProgram> m_shader;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_count = 0;
};

}  // namespace vlt
