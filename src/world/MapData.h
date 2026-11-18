#pragma once

#include <string>
#include <vector>

namespace vlt {

// Metres, Y up, -Z forward (docs/MAP_SCHEMA.md).
struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

inline bool operator==(const Vec3& lhs, const Vec3& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!=(const Vec3& lhs, const Vec3& rhs) {
    return !(lhs == rhs);
}

struct MapMetadata {
    std::string nameVi;
    std::string nameEn;
    std::string location;
    std::string description;
};

struct MapSkybox {
    std::string day;
    std::string dusk;
    std::string night;
};

struct MapStartCamera {
    Vec3 position;
    float yawDegrees = 0.0f;
    float pitchDegrees = 0.0f;  // clamped to [-89, 89] by the loader
};

struct MapViewpoint {
    int key = 0;
    std::string nameVi;
    Vec3 position;
    Vec3 target;
};

struct MapPart {
    std::string id;
    std::string node;
    std::string nameVi;
    std::string nameEn;
    std::string description;
};

struct MapHotspot {
    std::string id;
    std::string nameVi;
    Vec3 position;
    std::string description;
    bool hasTarget = false;
    Vec3 target;
};

struct MapData {
    int schemaVersion = 0;
    std::string id;
    MapMetadata metadata;
    std::string model;
    bool hasSkybox = false;
    MapSkybox skybox;
    MapStartCamera startCamera;
    std::vector<MapViewpoint> viewpoints;
    std::vector<Vec3> tourPath;
    std::vector<MapPart> parts;
    std::vector<MapHotspot> hotspots;
};

}  // namespace vlt
