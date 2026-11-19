#pragma once

#include <glm/glm.hpp>

#include <optional>

namespace vlt {

struct Ray {
    glm::vec3 origin{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
};

// Returns the nearest non-negative distance along a normalized ray.
std::optional<float> intersectRayAabb(const Ray& ray, const glm::vec3& minimum,
                                      const glm::vec3& maximum);
std::optional<float> intersectRaySphere(const Ray& ray, const glm::vec3& center,
                                        float radius);

}  // namespace vlt
