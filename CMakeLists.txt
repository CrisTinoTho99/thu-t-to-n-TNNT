cmake_minimum_required(VERSION 3.22)

project(VirtualLandmarkTour
    VERSION 0.1.0
    DESCRIPTION "Virtual landmark tour application (C++17 / OpenGL 3.3)"
    LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# Build settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# Keep single-config and multi-config generators in predictable folders.
foreach(config IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/bin")
endforeach()

# ---------------------------------------------------------------------------
# Dependencies
#
# See docs/BUILD.md for dependency versions and reasons. Runtime dependencies
# are pinned and fetched at configure time; to build offline, point CMake at
# existing checkouts with FETCHCONTENT_SOURCE_DIR_<NAME>.
# ---------------------------------------------------------------------------
include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

# GLAD 2.0.8 loader for OpenGL 3.3 core, generated ahead of time and vendored in
# external/glad (see external/glad/README.md for the exact command). Generating
# it with FetchContent at configure time would require Python + jinja2 on every
# developer machine, so the reproducible output is committed instead.
add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include")

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# JSON parser used by the shared map manifest loader (pinned, header-only).
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Assimp is built with only the glTF importer enabled to keep the shared model
# loader focused and reduce build time for contributors.
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_USE_CCACHE OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_IGNORE_GIT_HASH ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.4.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(assimp)

# ---------------------------------------------------------------------------
# Shared world data (map manifest loader, no OpenGL dependency)
# ---------------------------------------------------------------------------
add_library(vlt_world STATIC
    src/world/MapLoader.cpp
)
target_include_directories(vlt_world PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(vlt_world PUBLIC nlohmann_json::nlohmann_json)

# CPU-side model import is independent of an OpenGL context so it can be
# validated by tests without opening a window.
add_library(vlt_model_assets STATIC
    src/graphics/ModelAsset.cpp
)
target_include_directories(vlt_model_assets PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(vlt_model_assets PUBLIC assimp glm::glm)

# ---------------------------------------------------------------------------
# Application target
# ---------------------------------------------------------------------------
set(VLT_SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Input.cpp
    src/core/Log.cpp
    src/core/Paths.cpp
    src/core/SceneManager.cpp
    src/core/Time.cpp
    src/core/Window.cpp
    src/graphics/Camera.cpp
    src/graphics/Model.cpp
    src/graphics/ShaderProgram.cpp
    src/graphics/TestCube.cpp
    src/scenes/MainMenuScene.cpp
    src/scenes/MapScene.cpp
)

add_executable(vlt ${VLT_SOURCES})

target_include_directories(vlt PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(vlt PRIVATE glfw glad glm::glm vlt_world vlt_model_assets)

# GLAD replaces the legacy GL declarations that GLFW would otherwise include.
target_compile_definitions(vlt PRIVATE GLFW_INCLUDE_NONE)

if(WIN32)
    # Static GLFW does not propagate the OpenGL import library.
    target_link_libraries(vlt PRIVATE opengl32)
endif()

if(MSVC)
    target_compile_options(vlt PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(vlt PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MSVC)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT vlt)
endif()

# ---------------------------------------------------------------------------
# Tests (tiny assert-based harness, no external framework)
# ---------------------------------------------------------------------------
enable_testing()

add_executable(vlt_map_tests tests/map_loader_tests.cpp)
target_link_libraries(vlt_map_tests PRIVATE vlt_world)

add_test(NAME map_loader
         COMMAND vlt_map_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets")

add_executable(vlt_model_tests tests/model_asset_tests.cpp)
target_link_libraries(vlt_model_tests PRIVATE vlt_model_assets vlt_world)

add_test(NAME model_asset
         COMMAND vlt_model_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets")
