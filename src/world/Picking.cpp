#include "world/Picking.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace vlt {

std::optional<float> intersectRayAabb(const Ray& ray,
                                      const glm::vec3& minimum,
                                      const glm::vec3& maximum) {
    if (glm::dot(ray.direction, ray.direction) < 1.0e-12f) {
        return std::nullopt;
    }

    float nearDistance = 0.0f;
    float farDistance = std::numeric_limits<float>::max();
    constexpr float kParallelEpsilon = 1.0e-6f;

    for (int axis = 0; axis < 3; ++axis) {
        const float origin = ray.origin[axis];
        const float direction = ray.direction[axis];
        if (std::abs(direction) < kParallelEpsilon) {
            if (origin < minimum[axis] || origin > maximum[axis]) {
                return std::nullopt;
            }
            continue;
        }

        float first = (minimum[axis] - origin) / direction;
        float second = (maximum[axis] - origin) / direction;
        if (first > second) {
            std::swap(first, second);
        }
        nearDistance = std::max(nearDistance, first);
        farDistance = std::min(farDistance, second);
        if (nearDistance > farDistance) {
            return std::nullopt;
        }
    }

    if (farDistance < 0.0f) {
        return std::nullopt;
    }
    return nearDistance;
}

std::optional<float> intersectRaySphere(const Ray& ray,
                                        const glm::vec3& center,
                                        float radius) {
    if (radius <= 0.0f ||
        glm::dot(ray.direction, ray.direction) < 1.0e-12f) {
        return std::nullopt;
    }
    const glm::vec3 direction = glm::normalize(ray.direction);
    const glm::vec3 toCenter = center - ray.origin;
    const float projection = glm::dot(toCenter, direction);
    if (projection < 0.0f && glm::dot(toCenter, toCenter) > radius * radius) {
        return std::nullopt;
    }
    const float perpendicularSquared =
        glm::dot(toCenter, toCenter) - projection * projection;
    const float radiusSquared = radius * radius;
    if (perpendicularSquared > radiusSquared) {
        return std::nullopt;
    }

    const float halfChord =
        std::sqrt(std::max(0.0f, radiusSquared - perpendicularSquared));
    const float nearDistance = projection - halfChord;
    return nearDistance >= 0.0f ? nearDistance : projection + halfChord;
}

}  // namespace vlt
