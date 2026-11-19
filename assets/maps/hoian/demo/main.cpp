// Standalone map entry point using the tour's native loader, window and GL runtime.
#include "core/Input.h"
#include "core/Window.h"
#include "graphics/GL.h"
#include "graphics/ModelAsset.h"
#include "graphics/ShaderProgram.h"
#include "world/MapLoader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#endif

namespace fs = std::filesystem;
using namespace vlt;
using glm::vec3;
using glm::mat4;

std::string readFile(const fs::path& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open " + path.string());
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

vec3 vec(Vec3 p) { return {p.x, p.y, p.z}; }

struct DrawMesh {
    GLuint vao = 0, buffers[2]{};
    GLsizei count;
    int material;
    mat4 transform;
    explicit DrawMesh(const ImportedMesh& mesh)
        : count(static_cast<GLsizei>(mesh.indices.size())),
          material(mesh.nodeName.size() >= 3 && mesh.nodeName[0] == 'H'
                       ? std::stoi(mesh.nodeName.substr(1, 2))
                       : (mesh.nodeName.rfind("ENV_Water", 0) == 0 ? 13 : 12)),
          transform(mesh.nodeTransform) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(2, buffers);
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);
        for (unsigned i = 0; i < 3; i++) {
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), reinterpret_cast<void*>(i * 3 * sizeof(float)));
        }
    }
    DrawMesh(const DrawMesh&) = delete;
    ~DrawMesh() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(2, buffers);
    }
    void draw() const {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }
};

struct Target {
    GLuint fbo = 0, texture = 0, depth = 0;
    bool shadow;
    explicit Target(bool depthOnly) : shadow(depthOnly) {
        glGenFramebuffers(1, &fbo);
        glGenTextures(1, &texture);
        if (!shadow) glGenRenderbuffers(1, &depth);
    }
    void resize(int w, int h) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, shadow ? GL_DEPTH_COMPONENT24 : GL_RGB8, w, h, 0,
                    shadow ? GL_DEPTH_COMPONENT : GL_RGB,
                    shadow ? GL_UNSIGNED_INT : GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, shadow ? GL_NEAREST : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, shadow ? GL_NEAREST : GL_LINEAR);
        for (auto p : {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T})
            glTexParameteri(GL_TEXTURE_2D, p, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, shadow ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texture, 0);
        if (shadow) {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        } else {
            glBindRenderbuffer(GL_RENDERBUFFER, depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Incomplete render target");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ~Target() {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture);
        if (depth) glDeleteRenderbuffers(1, &depth);
    }
};

const char* skyVertex = R"(#version 330 core
out vec2 uv;
void main(){uv=vec2((gl_VertexID<<1)&2,gl_VertexID&2)*2.-1.;gl_Position=vec4(uv,.9999,1);})";

const char* skyFrag = R"(#version 330 core
in vec2 uv;out vec4 pixel;uniform mat4 invVP;uniform vec3 eye,sun;
void main(){vec4 p=invVP*vec4(uv,.99,1);vec3 ray=normalize(p.xyz/p.w-eye);
float h=max(ray.y,0.);float glow=pow(max(dot(ray,sun),0.),14.);
vec3 sky=mix(vec3(.94,.90,.81),vec3(.51,.71,.83),smoothstep(-.06,.56,h));
sky+=vec3(.07,.045,.011)*glow;pixel=vec4(sky,1);})";

const char* depthVertex = R"(#version 330 core
layout(location=0)in vec3 pos;uniform mat4 mvp;void main(){gl_Position=mvp*vec4(pos,1);})";

const char* depthFrag = R"(#version 330 core
void main(){})";

void capturePixels(const fs::path& path, int w, int h) {
    std::vector<unsigned char> data(w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << w << " " << h << "\n255\n";
    for (int y = h - 1; y >= 0; y--)
        out.write(reinterpret_cast<char*>(data.data() + y * w * 3), w * 3);
    if (!out) throw std::runtime_error("Could not write " + path.string());
}

struct Walker {
    vec3 pos{};
    float yaw = 0, pitch = 0;
    double mx = 0, my = 0;
    bool looking = false;
    vec3 forward() const {
        return {std::sin(yaw) * std::cos(pitch), std::sin(pitch), -std::cos(yaw) * std::cos(pitch)};
    }
    void jump(const MapViewpoint& view) {
        pos = vec(view.position);
        auto f = glm::normalize(vec(view.target) - pos);
        yaw = std::atan2(f.x, -f.z);
        pitch = std::asin(f.y);
    }
    bool walkable(vec3 p) const {
        if (std::abs(p.x) > 23 || p.z < -29 || p.z > 23) return false;
        if (p.x >= 3.53f && p.x <= 6.42f && std::abs(p.z-6.15f) < .98f) return true;
        // Keep the street open between the new shopfronts at either portal.
        if (std::abs(p.x) >= 10.f && std::abs(p.z) > 3.12f && std::abs(p.z) < 8.65f) return false;
        if (std::abs(p.x) < 9.42f && std::abs(p.z) < 1.04f) return true;
        if (std::abs(p.x) < 0.69f && p.z >= -4.55f && p.z < 0) return true;
        if (std::abs(p.x) < 1.86f && p.z >= -4.55f && p.z < -1.76f) return true;
        if (std::abs(p.x) < 9.24f && std::abs(p.z) < 1.88f) return false;
        if (p.z >= 16.55f && p.z <= 21.65f) return true;
        return std::abs(p.x) > 6.39f && (p.z > 2.f || std::abs(p.x) > 9.24f);
    }
    void update(const Input& in, float dt) {
        if (in.isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (looking) {
                yaw += float(in.mouseX() - mx) * 0.003f;
                pitch = std::clamp(pitch - float(in.mouseY() - my) * 0.003f, -1.48f, 1.48f);
            }
            looking = true;
        } else looking = false;
        mx = in.mouseX();
        my = in.mouseY();
        vec3 f(std::sin(yaw), 0, -std::cos(yaw)), r(-f.z, 0, f.x);
        vec3 delta = f * float(in.isKeyDown(GLFW_KEY_W) - in.isKeyDown(GLFW_KEY_S)) +
                     r * float(in.isKeyDown(GLFW_KEY_D) - in.isKeyDown(GLFW_KEY_A));
        if (glm::length(delta) < 0.01f) return;
        delta = glm::normalize(delta) * dt * (in.isKeyDown(GLFW_KEY_LEFT_SHIFT) ? 3.5f : 1.65f);
        vec3 next = pos;
        next.x += delta.x;
        if (walkable(next)) pos.x = next.x;
        next = pos;
        next.z += delta.z;
        if (walkable(next)) pos.z = next.z;
        pos.y = 2.76f;
        if (pos.x >= 3.53f && pos.x < 6.19f && std::abs(pos.z-6.15f)<.98f)
            pos.y = 1.80f + .16f*std::clamp(std::floor((pos.x-4.15f)/.34f),0.f,5.f);
        else if (std::abs(pos.z) < 1.55f && std::abs(pos.x) < 9)
            pos.y += 0.34f * (1 - pos.x * pos.x / 81);
        else if (pos.z < -1.55f && std::abs(pos.x) < 1.86f)
            pos.y = 3.11f;
    }
};

float viewFov(int key, int w, int h) {
    const float aspect = float(w)/h;
    if (key == 1)
        return 2.f*std::atan(std::tan(glm::radians(64.f)*.5f)/aspect);
    float vertical = key == 2 ? 60.f : key == 3 ? 48.f : 57.f;
    // Preserve a readable plaque and the diagonal row of piers on portrait screens.
    float horizontalMin = key == 3 ? 49.f : 54.f;
    return std::max(glm::radians(vertical),
                    2.f*std::atan(std::tan(glm::radians(horizontalMin)*.5f)/aspect));
}

void renderView(int w, int h, const vec3& eye, const vec3& fwd, float fovY,
                GLuint destFbo, Target& mirrorTarget, Target& shadowTarget,
                ShaderProgram& sceneProg, ShaderProgram& skyProg,
                GLuint skyVAO, const std::vector<std::unique_ptr<DrawMesh>>& meshes,
                const mat4& lightSpace, const vec3& sun, double timeSec) {
    mirrorTarget.resize(w, h);
    auto proj = glm::perspective(fovY, float(w) / h, 0.035f, 140.0f);
    auto view = glm::lookAt(eye, eye + fwd, vec3(0, 1, 0));
    auto flip = glm::translate(mat4(1), vec3(0, -0.46f, 0)) * glm::scale(mat4(1), vec3(1, -1, 1));

    for (int pass = 0; pass < 2; pass++) {
        bool reflected = (pass == 0);
        auto vp = proj * view * (reflected ? flip : mat4(1));
        vec3 curEye = reflected ? vec3(flip * glm::vec4(eye, 1)) : eye;

        glBindFramebuffer(GL_FRAMEBUFFER, reflected ? mirrorTarget.fbo : destFbo);
        glViewport(0, 0, w, h);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        skyProg.use();
        skyProg.setMat4("invVP", glm::inverse(vp));
        skyProg.setVec3("eye", curEye);
        skyProg.setVec3("sun", sun);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glEnable(GL_DEPTH_TEST);
        sceneProg.use();
        sceneProg.setMat4("vp", vp);
        sceneProg.setMat4("lightSpace", lightSpace);
        sceneProg.setVec3("sun", sun);
        sceneProg.setVec3("eye", curEye);
        sceneProg.setVec3("frame", vec3(w, h, timeSec));
        sceneProg.setInt("mirrored", reflected);
        sceneProg.setInt("shadowDepth", 0);
        sceneProg.setInt("mirrorColor", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowTarget.texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, reflected ? 0 : mirrorTarget.texture);

        for (auto& m : meshes) {
            if (reflected && m->material == 13) continue;
            sceneProg.setInt("material", m->material);
            sceneProg.setMat4("model", m->transform);
            m->draw();
        }
    }
}

int run(const fs::path& package, const fs::path& captures) {
    MapLoadOptions options;
    options.verifyAssetFiles = true;
    auto result = loadMapManifest(package / "map.json", package.parent_path().parent_path(), "hoian", options);
    if (!result.ok) {
        for (auto& e : result.errors) std::cerr << e << '\n';
        return 1;
    }
    auto& map = result.data;
    Input input;
    Window window(1280, 720, "Chùa Cầu · Hội An   |   1 / 2 / 3 / 4   ·   WASD   ·   RMB", input);
    glfwShowWindow(window.handle());
    glfwFocusWindow(window.handle());
#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(window.handle());
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOWNORMAL);
        SetForegroundWindow(hwnd);
        BringWindowToTop(hwnd);
        SetActiveWindow(hwnd);
    }
#endif
    auto imported = loadModelAsset(package / map.model);
    for (auto& p : map.parts) {
        if (!imported.hasUniqueNode(p.node))
            throw std::runtime_error("Missing or ambiguous part: " + p.node);
    }
    std::vector<std::unique_ptr<DrawMesh>> meshes;
    for (auto& m : imported.meshes) meshes.emplace_back(std::make_unique<DrawMesh>(m));
    std::cout << "Loaded " << map.id << ": " << meshes.size() << " meshes, " << map.parts.size() << " verified parts.\n";

    auto sv = readFile(package / "demo/scene.vert");
    auto sf = readFile(package / "demo/scene.frag");
    ShaderProgram sceneProg(sv.c_str(), sf.c_str());
    ShaderProgram skyProg(skyVertex, skyFrag);
    ShaderProgram depthProg(depthVertex, depthFrag);

    Target shadowTarget(true);
    shadowTarget.resize(2048, 2048);
    Target mirrorTarget(false);

    vec3 sun = glm::normalize(vec3(0.81f, 0.55f, 0.41f));
    auto light = glm::ortho(-22.0f, 22.0f, -19.0f, 19.0f, 0.1f, 95.0f) *
                 glm::lookAt(sun * 45.0f, vec3(0), vec3(0, 1, 0));

    glBindFramebuffer(GL_FRAMEBUFFER, shadowTarget.fbo);
    glViewport(0, 0, 2048, 2048);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthProg.use();
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.3f, 2.1f);
    for (auto& m : meshes) {
        if (m->material != 13) {
            depthProg.setMat4("mvp", light * m->transform);
            m->draw();
        }
    }
    glDisable(GL_POLYGON_OFFSET_FILL);

    GLuint skyVAO;
    glGenVertexArrays(1, &skyVAO);

    // If capture mode is requested, render both Desktop and Mobile resolutions
    if (!captures.empty()) {
        fs::create_directories(captures);
        std::cout << "Rendering offline QA captures to " << captures.string() << "...\n";

        struct CaptureProfile {
            std::string tag;
            int width, height;
        };
        std::vector<CaptureProfile> profiles = {
            {"desktop", 1920, 1080},
            {"mobile", 390, 844}
        };

        Target offscreenTarget(false);

        for (const auto& prof : profiles) {
            int w = prof.width;
            int h = prof.height;
            offscreenTarget.resize(w, h);

            for (size_t i = 0; i < map.viewpoints.size(); ++i) {
                const auto& vp = map.viewpoints[i];
                vec3 eye = vec(vp.position);
                vec3 fwd = glm::normalize(vec(vp.target) - eye);

                float fovY = viewFov(vp.key,w,h);

                renderView(w, h, eye, fwd, fovY, offscreenTarget.fbo, mirrorTarget, shadowTarget,
                           sceneProg, skyProg, skyVAO, meshes, light, sun, 1.0);

                glBindFramebuffer(GL_FRAMEBUFFER, offscreenTarget.fbo);
                auto outPath = captures / ("view" + std::to_string(i + 1) + "_" + prof.tag + ".ppm");
                capturePixels(outPath, w, h);
                std::cout << "  Saved [" << prof.tag << "] " << vp.nameVi << " -> " << outPath.string() << "\n";

                if (prof.tag == "desktop") {
                    auto legacyPath = captures / (std::to_string(i + 1) + ".ppm");
                    capturePixels(legacyPath, w, h);
                }
            }
        }
        std::cout << "All QA captures saved successfully.\n";
        glDeleteVertexArrays(1, &skyVAO);
        return 0;
    }

    Walker walker;
    walker.jump(map.viewpoints.at(0));
    int viewKey = map.viewpoints.at(0).key;
    double prev = glfwGetTime();

    while (!window.shouldClose()) {
        window.pollEvents();
        if (input.wasKeyPressed(GLFW_KEY_ESCAPE)) break;
        for (auto& v : map.viewpoints)
            if (input.wasKeyPressed(GLFW_KEY_0 + v.key)) {
                walker.jump(v);
                viewKey = v.key;
            }

        double now = glfwGetTime();
        walker.update(input, float(std::min(0.05, now - prev)));
        prev = now;

        int w = window.framebufferWidth(), h = window.framebufferHeight();
        if (w <= 0 || h <= 0) {
            glfwWaitEventsTimeout(0.1);
            continue;
        }

        float fovY = viewFov(viewKey,w,h);
        renderView(w, h, walker.pos, walker.forward(), fovY, 0, mirrorTarget, shadowTarget,
                   sceneProg, skyProg, skyVAO, meshes, light, sun, now);

        window.swapBuffers();
        input.endFrame();
    }
    glDeleteVertexArrays(1, &skyVAO);
    return 0;
}

int main(int argc, char** argv) {
    try {
        fs::path package = argc > 1 ? fs::absolute(argv[1]) : fs::current_path();
        if (argc == 1 && !fs::exists(package / "map.json"))
            package = fs::absolute(argv[0]).parent_path().parent_path().parent_path() / "assets/maps/hoian";
        return run(package, argc == 4 && std::string(argv[2]) == "--capture" ? fs::absolute(argv[3]) : fs::path{});
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
