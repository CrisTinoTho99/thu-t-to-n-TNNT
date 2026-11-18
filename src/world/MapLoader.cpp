#include "world/MapLoader.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <regex>
#include <set>
#include <string_view>
#include <system_error>

namespace vlt {
namespace {

using json = nlohmann::json;

constexpr int kSupportedSchemaVersion = 1;
constexpr std::string_view kCommonPrefix = "common://";

bool startsWith(std::string_view value, std::string_view prefix) {
    return value.size() >= prefix.size() &&
           value.compare(0, prefix.size(), prefix) == 0;
}

void addError(MapLoadResult& result, const std::string& mapId,
              const std::string& pointer, const std::string& message) {
    std::string text = "Map '" + mapId + "': ";
    if (!pointer.empty()) {
        text += pointer + ": ";
    }
    text += message;
    result.errors.push_back(text);
}

bool validateReference(const std::string& value, std::string& reason) {
    if (value.empty()) {
        reason = "asset path must not be empty";
        return false;
    }

    std::string relative = value;
    if (startsWith(value, kCommonPrefix)) {
        relative = value.substr(kCommonPrefix.size());
        if (relative.empty()) {
            reason = "common:// path must name a file or directory";
            return false;
        }
    } else if (value.find("://") != std::string::npos) {
        reason = "unsupported URI scheme; only common:// is allowed";
        return false;
    }

    if (relative.front() == '/') {
        reason = "absolute paths are not allowed";
        return false;
    }
    if (relative.size() >= 2 &&
        std::isalpha(static_cast<unsigned char>(relative[0])) != 0 &&
        relative[1] == ':') {
        reason = "drive-letter paths are not allowed";
        return false;
    }
    if (relative.find('\\') != std::string::npos) {
        reason = "use '/' as the path separator";
        return false;
    }
    if (relative.find('~') != std::string::npos) {
        reason = "'~' is not allowed in asset paths";
        return false;
    }
    if (relative.find('%') != std::string::npos ||
        relative.find('$') != std::string::npos) {
        reason = "environment variable expansion is not allowed";
        return false;
    }
    if (relative.find(':') != std::string::npos) {
        reason = "':' is not allowed in asset paths";
        return false;
    }

    std::size_t segmentStart = 0;
    while (segmentStart <= relative.size()) {
        const std::size_t slash = relative.find('/', segmentStart);
        const std::size_t segmentEnd =
            slash == std::string::npos ? relative.size() : slash;
        const std::string_view segment(relative.data() + segmentStart,
                                       segmentEnd - segmentStart);
        if (segment.empty()) {
            reason = "empty path segment (check for '//' or a trailing '/')";
            return false;
        }
        if (segment == "." || segment == "..") {
            reason = "'.' and '..' segments are not allowed";
            return false;
        }
        if (slash == std::string::npos) {
            break;
        }
        segmentStart = slash + 1;
    }

    const std::size_t dot = relative.find_last_of('.');
    const std::size_t slash = relative.find_last_of('/');
    if (dot != std::string::npos && (slash == std::string::npos || dot > slash) &&
        dot + 1 < relative.size()) {
        const std::string extension = relative.substr(dot + 1);
        for (const char character : extension) {
            if (std::isupper(static_cast<unsigned char>(character)) != 0) {
                reason = "file extensions must be lowercase";
                return false;
            }
        }
    }

    return true;
}

std::string pointerJoin(const std::string& base, const std::string& key) {
    return base + "/" + key;
}

void checkUnknownKeys(const json& object,
                      std::initializer_list<const char*> allowed,
                      const std::string& base, MapLoadResult& result,
                      const std::string& mapId) {
    for (auto it = object.begin(); it != object.end(); ++it) {
        bool known = false;
        for (const char* key : allowed) {
            if (it.key() == key) {
                known = true;
                break;
            }
        }
        if (!known) {
            addError(result, mapId, pointerJoin(base, it.key()),
                     "unknown field");
        }
    }
}

std::optional<std::string> readString(const json& object, const char* key,
                                      const std::string& base, bool required,
                                      MapLoadResult& result,
                                      const std::string& mapId,
                                      bool allowEmpty = false) {
    const std::string pointer = pointerJoin(base, key);
    const auto it = object.find(key);
    if (it == object.end()) {
        if (required) {
            addError(result, mapId, pointer, "required field is missing");
        }
        return std::nullopt;
    }
    if (!it->is_string()) {
        addError(result, mapId, pointer, "must be a string");
        return std::nullopt;
    }
    std::string value = it->get<std::string>();
    if (!allowEmpty && value.empty()) {
        addError(result, mapId, pointer, "must not be empty");
        return std::nullopt;
    }
    return value;
}

std::optional<long long> readInteger(const json& object, const char* key,
                                     const std::string& base,
                                     MapLoadResult& result,
                                     const std::string& mapId) {
    const std::string pointer = pointerJoin(base, key);
    const auto it = object.find(key);
    if (it == object.end()) {
        addError(result, mapId, pointer, "required field is missing");
        return std::nullopt;
    }
    if (!it->is_number_integer()) {
        addError(result, mapId, pointer, "must be an integer");
        return std::nullopt;
    }
    return it->get<long long>();
}

std::optional<double> readNumber(const json& object, const char* key,
                                 const std::string& base,
                                 MapLoadResult& result,
                                 const std::string& mapId) {
    const std::string pointer = pointerJoin(base, key);
    const auto it = object.find(key);
    if (it == object.end()) {
        addError(result, mapId, pointer, "required field is missing");
        return std::nullopt;
    }
    if (!it->is_number()) {
        addError(result, mapId, pointer, "must be a number");
        return std::nullopt;
    }
    const double value = it->get<double>();
    if (!std::isfinite(value)) {
        addError(result, mapId, pointer, "must be a finite number");
        return std::nullopt;
    }
    return value;
}

std::optional<Vec3> readVec3Value(const json& value, const std::string& pointer,
                                  MapLoadResult& result,
                                  const std::string& mapId) {
    if (!value.is_array() || value.size() != 3) {
        addError(result, mapId, pointer,
                 "must be an array of exactly 3 numbers");
        return std::nullopt;
    }

    Vec3 vector;
    float* components[3] = {&vector.x, &vector.y, &vector.z};
    bool ok = true;
    for (std::size_t index = 0; index < 3; ++index) {
        const json& component = value[index];
        const std::string componentPointer =
            pointer + "/" + std::to_string(index);
        if (!component.is_number()) {
            addError(result, mapId, componentPointer, "must be a number");
            ok = false;
            continue;
        }
        const double number = component.get<double>();
        if (!std::isfinite(number)) {
            addError(result, mapId, componentPointer,
                     "must be a finite number");
            ok = false;
            continue;
        }
        *components[index] = static_cast<float>(number);
    }
    if (!ok) {
        return std::nullopt;
    }
    return vector;
}

std::optional<Vec3> readVec3(const json& object, const char* key,
                             const std::string& base, MapLoadResult& result,
                             const std::string& mapId) {
    const std::string pointer = pointerJoin(base, key);
    const auto it = object.find(key);
    if (it == object.end()) {
        addError(result, mapId, pointer, "required field is missing");
        return std::nullopt;
    }
    return readVec3Value(*it, pointer, result, mapId);
}

bool isIdPattern(const std::string& value, const std::regex& pattern) {
    return std::regex_match(value, pattern);
}

std::string describeJsonPosition(const std::string& text, std::size_t byte) {
    std::size_t line = 1;
    std::size_t column = 1;
    const std::size_t limit = std::min(byte, text.size());
    for (std::size_t index = 0; index < limit; ++index) {
        if (text[index] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
    return "line " + std::to_string(line) + ", column " +
           std::to_string(column);
}

}  // namespace

std::filesystem::path resolveMapAsset(const std::filesystem::path& assetRoot,
                                      const std::string& mapId,
                                      const std::string& reference) {
    std::string reason;
    if (!validateReference(reference, reason)) {
        return {};
    }

    if (startsWith(reference, kCommonPrefix)) {
        return assetRoot / "common" / reference.substr(kCommonPrefix.size());
    }
    return assetRoot / "maps" / mapId / reference;
}

MapLoadResult loadMapManifest(const std::filesystem::path& manifestPath,
                              const std::filesystem::path& assetRoot,
                              const std::string& expectedMapId,
                              const MapLoadOptions& options) {
    MapLoadResult result;
    result.manifestPath = manifestPath;

    std::error_code fileError;
    if (!std::filesystem::is_regular_file(manifestPath, fileError)) {
        addError(result, expectedMapId, "",
                 "manifest not found at '" + manifestPath.string() + "'");
        return result;
    }

    std::ifstream stream(manifestPath, std::ios::binary);
    if (!stream) {
        addError(result, expectedMapId, "",
                 "cannot open manifest at '" + manifestPath.string() + "'");
        return result;
    }
    const std::string text((std::istreambuf_iterator<char>(stream)),
                           std::istreambuf_iterator<char>());
    if (text.size() >= 3 &&
        static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB &&
        static_cast<unsigned char>(text[2]) == 0xBF) {
        addError(result, expectedMapId, "",
                 "file must be UTF-8 without a BOM");
        return result;
    }

    json root;
    try {
        root = json::parse(text);
    } catch (const json::parse_error& error) {
        const std::size_t byte =
            error.byte > 0 ? static_cast<std::size_t>(error.byte - 1) : 0;
        addError(result, expectedMapId, "",
                 "JSON parse error at " + describeJsonPosition(text, byte) +
                     ": " + error.what());
        return result;
    }

    if (!root.is_object()) {
        addError(result, expectedMapId, "", "root must be a JSON object");
        return result;
    }

    checkUnknownKeys(root,
                     {"schema_version", "id", "metadata", "model", "skybox",
                      "start_camera", "viewpoints", "tour_path", "parts",
                      "hotspots"},
                     "", result, expectedMapId);

    // schema_version
    if (!root.contains("schema_version")) {
        addError(result, expectedMapId, "/schema_version",
                 "required field is missing");
    } else if (!root.at("schema_version").is_number_integer()) {
        addError(result, expectedMapId, "/schema_version",
                 "must be an integer");
    } else {
        const long long version = root.at("schema_version").get<long long>();
        if (version != kSupportedSchemaVersion) {
            addError(result, expectedMapId, "/schema_version",
                     "unsupported schema_version " + std::to_string(version) +
                         " (expected " +
                         std::to_string(kSupportedSchemaVersion) + ")");
        } else {
            result.data.schemaVersion = static_cast<int>(version);
        }
    }

    // id
    static const std::regex kMapIdPattern("^[a-z][a-z0-9_]{1,31}$");
    if (const auto id =
            readString(root, "id", "", true, result, expectedMapId)) {
        if (!isIdPattern(*id, kMapIdPattern)) {
            addError(result, expectedMapId, "/id",
                     "must match ^[a-z][a-z0-9_]{1,31}$");
        } else if (*id != expectedMapId) {
            addError(result, expectedMapId, "/id",
                     "must match the package directory name '" + expectedMapId +
                         "'");
        } else {
            result.data.id = *id;
        }
    }

    // metadata
    if (!root.contains("metadata")) {
        addError(result, expectedMapId, "/metadata",
                 "required field is missing");
    } else if (!root.at("metadata").is_object()) {
        addError(result, expectedMapId, "/metadata", "must be an object");
    } else {
        const json& metadata = root.at("metadata");
        checkUnknownKeys(metadata, {"name_vi", "name_en", "location",
                                    "description"},
                         "/metadata", result, expectedMapId);
        if (const auto value = readString(metadata, "name_vi", "/metadata", true,
                                          result, expectedMapId)) {
            result.data.metadata.nameVi = *value;
        }
        if (const auto value = readString(metadata, "name_en", "/metadata", true,
                                          result, expectedMapId)) {
            result.data.metadata.nameEn = *value;
        }
        if (const auto value = readString(metadata, "location", "/metadata",
                                          true, result, expectedMapId)) {
            result.data.metadata.location = *value;
        }
        if (const auto value = readString(metadata, "description", "/metadata",
                                          false, result, expectedMapId,
                                          true)) {
            result.data.metadata.description = *value;
        }
    }

    // model
    if (const auto model =
            readString(root, "model", "", true, result, expectedMapId)) {
        std::string reason;
        if (!validateReference(*model, reason)) {
            addError(result, expectedMapId, "/model", reason);
        } else {
            result.data.model = *model;
        }
    }

    // skybox (optional)
    if (root.contains("skybox")) {
        if (!root.at("skybox").is_object()) {
            addError(result, expectedMapId, "/skybox", "must be an object");
        } else {
            const json& skybox = root.at("skybox");
            checkUnknownKeys(skybox, {"day", "dusk", "night"}, "/skybox",
                             result, expectedMapId);
            const auto day = readString(skybox, "day", "/skybox", true, result,
                                        expectedMapId);
            const auto dusk = readString(skybox, "dusk", "/skybox", true, result,
                                         expectedMapId);
            const auto night = readString(skybox, "night", "/skybox", true,
                                          result, expectedMapId);
            if (day && dusk && night) {
                bool valid = true;
                std::string reason;
                if (!validateReference(*day, reason)) {
                    addError(result, expectedMapId, "/skybox/day", reason);
                    valid = false;
                }
                if (!validateReference(*dusk, reason)) {
                    addError(result, expectedMapId, "/skybox/dusk", reason);
                    valid = false;
                }
                if (!validateReference(*night, reason)) {
                    addError(result, expectedMapId, "/skybox/night", reason);
                    valid = false;
                }
                if (valid) {
                    result.data.hasSkybox = true;
                    result.data.skybox = {*day, *dusk, *night};
                }
            }
        }
    }

    // start_camera
    if (!root.contains("start_camera")) {
        addError(result, expectedMapId, "/start_camera",
                 "required field is missing");
    } else if (!root.at("start_camera").is_object()) {
        addError(result, expectedMapId, "/start_camera",
                 "must be an object");
    } else {
        const json& camera = root.at("start_camera");
        checkUnknownKeys(camera, {"position", "yaw", "pitch"}, "/start_camera",
                         result, expectedMapId);
        if (const auto position =
                readVec3(camera, "position", "/start_camera", result,
                         expectedMapId)) {
            result.data.startCamera.position = *position;
        }
        if (const auto yaw =
                readNumber(camera, "yaw", "/start_camera", result,
                           expectedMapId)) {
            result.data.startCamera.yawDegrees = static_cast<float>(*yaw);
        }
        if (const auto pitch =
                readNumber(camera, "pitch", "/start_camera", result,
                           expectedMapId)) {
            // Schema requirement: clamp pitch to [-89, 89] degrees.
            result.data.startCamera.pitchDegrees =
                std::clamp(static_cast<float>(*pitch), -89.0f, 89.0f);
        }
    }

    // viewpoints
    if (!root.contains("viewpoints")) {
        addError(result, expectedMapId, "/viewpoints",
                 "required field is missing");
    } else if (!root.at("viewpoints").is_array()) {
        addError(result, expectedMapId, "/viewpoints",
                 "must be an array");
    } else {
        const json& viewpoints = root.at("viewpoints");
        if (viewpoints.empty() || viewpoints.size() > 9) {
            addError(result, expectedMapId, "/viewpoints",
                     "must contain between 1 and 9 viewpoints");
        }
        std::set<int> keys;
        for (std::size_t index = 0; index < viewpoints.size(); ++index) {
            const std::string pointer =
                "/viewpoints/" + std::to_string(index);
            const json& item = viewpoints[index];
            if (!item.is_object()) {
                addError(result, expectedMapId, pointer,
                         "must be an object");
                continue;
            }
            checkUnknownKeys(item, {"key", "name_vi", "position", "target"},
                             pointer, result, expectedMapId);

            MapViewpoint viewpoint;
            bool valid = true;
            if (const auto key =
                    readInteger(item, "key", pointer, result, expectedMapId)) {
                if (*key < 1 || *key > 9) {
                    addError(result, expectedMapId, pointer + "/key",
                             "must be between 1 and 9");
                    valid = false;
                } else if (!keys.insert(static_cast<int>(*key)).second) {
                    addError(result, expectedMapId, pointer + "/key",
                             "duplicate viewpoint key " +
                                 std::to_string(*key));
                    valid = false;
                } else {
                    viewpoint.key = static_cast<int>(*key);
                }
            } else {
                valid = false;
            }
            if (const auto name =
                    readString(item, "name_vi", pointer, true, result,
                               expectedMapId)) {
                viewpoint.nameVi = *name;
            } else {
                valid = false;
            }
            if (const auto position =
                    readVec3(item, "position", pointer, result,
                             expectedMapId)) {
                viewpoint.position = *position;
            } else {
                valid = false;
            }
            if (const auto target =
                    readVec3(item, "target", pointer, result, expectedMapId)) {
                viewpoint.target = *target;
                if (viewpoint.position == viewpoint.target) {
                    addError(result, expectedMapId, pointer + "/target",
                             "must differ from position");
                    valid = false;
                }
            } else {
                valid = false;
            }

            if (valid) {
                result.data.viewpoints.push_back(viewpoint);
            }
        }
    }

    // tour_path
    if (!root.contains("tour_path")) {
        addError(result, expectedMapId, "/tour_path",
                 "required field is missing");
    } else if (!root.at("tour_path").is_array()) {
        addError(result, expectedMapId, "/tour_path", "must be an array");
    } else {
        const json& tourPath = root.at("tour_path");
        if (tourPath.size() < 2 || tourPath.size() > 64) {
            addError(result, expectedMapId, "/tour_path",
                     "must contain between 2 and 64 points");
        }
        std::optional<Vec3> previous;
        for (std::size_t index = 0; index < tourPath.size(); ++index) {
            const std::string pointer =
                "/tour_path/" + std::to_string(index);
            const auto point =
                readVec3Value(tourPath[index], pointer, result, expectedMapId);
            if (!point) {
                previous.reset();
                continue;
            }
            if (previous && *previous == *point) {
                addError(result, expectedMapId, pointer,
                         "consecutive tour points must differ");
            }
            result.data.tourPath.push_back(*point);
            previous = *point;
        }
    }

    // parts
    static const std::regex kEntityIdPattern("^[A-Za-z][A-Za-z0-9_]*$");
    std::set<std::string> entityIds;
    if (!root.contains("parts")) {
        addError(result, expectedMapId, "/parts",
                 "required field is missing");
    } else if (!root.at("parts").is_array()) {
        addError(result, expectedMapId, "/parts", "must be an array");
    } else {
        const json& parts = root.at("parts");
        for (std::size_t index = 0; index < parts.size(); ++index) {
            const std::string pointer = "/parts/" + std::to_string(index);
            const json& item = parts[index];
            if (!item.is_object()) {
                addError(result, expectedMapId, pointer,
                         "must be an object");
                continue;
            }
            checkUnknownKeys(
                item, {"id", "node", "name_vi", "name_en", "description"},
                pointer, result, expectedMapId);

            MapPart part;
            bool valid = true;
            if (const auto id =
                    readString(item, "id", pointer, true, result,
                               expectedMapId)) {
                if (!isIdPattern(*id, kEntityIdPattern)) {
                    addError(result, expectedMapId, pointer + "/id",
                             "must match ^[A-Za-z][A-Za-z0-9_]*$");
                    valid = false;
                } else if (!entityIds.insert(*id).second) {
                    addError(result, expectedMapId, pointer + "/id",
                             "duplicate id '" + *id + "'");
                    valid = false;
                } else {
                    part.id = *id;
                }
            } else {
                valid = false;
            }
            if (const auto node =
                    readString(item, "node", pointer, true, result,
                               expectedMapId)) {
                part.node = *node;
            } else {
                valid = false;
            }
            if (const auto name =
                    readString(item, "name_vi", pointer, true, result,
                               expectedMapId)) {
                part.nameVi = *name;
            } else {
                valid = false;
            }
            if (const auto name =
                    readString(item, "name_en", pointer, false, result,
                               expectedMapId, true)) {
                part.nameEn = *name;
            }
            if (const auto description =
                    readString(item, "description", pointer, true, result,
                               expectedMapId)) {
                part.description = *description;
            } else {
                valid = false;
            }

            if (valid) {
                result.data.parts.push_back(part);
            }
        }
    }

    // hotspots
    if (!root.contains("hotspots")) {
        addError(result, expectedMapId, "/hotspots",
                 "required field is missing");
    } else if (!root.at("hotspots").is_array()) {
        addError(result, expectedMapId, "/hotspots", "must be an array");
    } else {
        const json& hotspots = root.at("hotspots");
        for (std::size_t index = 0; index < hotspots.size(); ++index) {
            const std::string pointer = "/hotspots/" + std::to_string(index);
            const json& item = hotspots[index];
            if (!item.is_object()) {
                addError(result, expectedMapId, pointer,
                         "must be an object");
                continue;
            }
            checkUnknownKeys(item,
                             {"id", "name_vi", "position", "description",
                              "target"},
                             pointer, result, expectedMapId);

            MapHotspot hotspot;
            bool valid = true;
            if (const auto id =
                    readString(item, "id", pointer, true, result,
                               expectedMapId)) {
                if (!isIdPattern(*id, kEntityIdPattern)) {
                    addError(result, expectedMapId, pointer + "/id",
                             "must match ^[A-Za-z][A-Za-z0-9_]*$");
                    valid = false;
                } else if (!entityIds.insert(*id).second) {
                    addError(result, expectedMapId, pointer + "/id",
                             "duplicate id '" + *id +
                                 "' (ids must be unique across parts and "
                                 "hotspots)");
                    valid = false;
                } else {
                    hotspot.id = *id;
                }
            } else {
                valid = false;
            }
            if (const auto name =
                    readString(item, "name_vi", pointer, true, result,
                               expectedMapId)) {
                hotspot.nameVi = *name;
            } else {
                valid = false;
            }
            if (const auto position =
                    readVec3(item, "position", pointer, result,
                             expectedMapId)) {
                hotspot.position = *position;
            } else {
                valid = false;
            }
            if (const auto description =
                    readString(item, "description", pointer, true, result,
                               expectedMapId)) {
                hotspot.description = *description;
            } else {
                valid = false;
            }
            if (item.contains("target")) {
                if (const auto target = readVec3(item, "target", pointer,
                                                 result, expectedMapId)) {
                    hotspot.hasTarget = true;
                    hotspot.target = *target;
                } else {
                    valid = false;
                }
            }

            if (valid) {
                result.data.hotspots.push_back(hotspot);
            }
        }
    }

    // Optional asset existence check, enabled once the asset pipeline exists.
    if (options.verifyAssetFiles && result.data.model.empty() == false) {
        const std::filesystem::path resolved =
            resolveMapAsset(assetRoot, expectedMapId, result.data.model);
        std::error_code assetError;
        if (resolved.empty() ||
            !std::filesystem::is_regular_file(resolved, assetError)) {
            addError(result, expectedMapId, "/model",
                     "referenced file does not exist at '" +
                         resolved.string() + "'");
        }
    }

    result.ok = result.errors.empty();
    return result;
}

}  // namespace vlt
