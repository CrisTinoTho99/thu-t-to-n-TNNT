#include <glm/glm.hpp>

#include <cmath>
#include <iostream>
#include <optional>
#include <string>

#include "world/Picking.h"

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& label) {
    ++g_checks;
    if (condition) {
        std::cout << "[PASS] " << label << '\n';
        return;
    }
    ++g_failures;
    std::cout << "[FAIL] " << label << '\n';
}

bool near(float actual, float expected) {
    return std::abs(actual - expected) < 1.0e-4f;
}

}  // namespace

int main() {
    const glm::vec3 minimum(-1.0f);
    const glm::vec3 maximum(1.0f);
    const auto hit = vlt::intersectRayAabb(
        {{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}}, minimum, maximum);
    check(hit && near(*hit, 4.0f), "ray hits the nearest AABB face");

    const auto inside = vlt::intersectRayAabb(
        {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, minimum, maximum);
    check(inside && near(*inside, 0.0f), "ray starting inside AABB hits at zero");

    const auto parallelHit = vlt::intersectRayAabb(
        {{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}}, minimum, maximum);
    check(parallelHit.has_value(), "parallel slab axes do not reject a valid hit");

    const auto miss = vlt::intersectRayAabb(
        {{3.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}}, minimum, maximum);
    check(!miss, "ray outside a parallel slab misses the AABB");

    const auto zeroDirection = vlt::intersectRayAabb(
        {{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f}}, minimum, maximum);
    check(!zeroDirection, "zero-length ray direction is rejected");

    const auto sphereHit = vlt::intersectRaySphere(
        {{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}}, glm::vec3(0.0f), 1.0f);
    check(sphereHit && near(*sphereHit, 4.0f), "ray hits hotspot sphere");

    const auto sphereTangent = vlt::intersectRaySphere(
        {{1.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}}, glm::vec3(0.0f), 1.0f);
    check(sphereTangent && near(*sphereTangent, 5.0f),
          "ray tangent to hotspot sphere counts as a hit");

    const auto sphereMiss = vlt::intersectRaySphere(
        {{2.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}}, glm::vec3(0.0f), 1.0f);
    check(!sphereMiss, "ray outside hotspot sphere misses");

    const auto invalidRadius = vlt::intersectRaySphere(
        {{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}}, glm::vec3(0.0f), 0.0f);
    check(!invalidRadius, "non-positive hotspot radius is rejected");

    std::cout << g_checks - g_failures << "/" << g_checks
              << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
