#pragma once

namespace vlt {

class Input;

// Common lifecycle for every scene. The scene manager guarantees that
// onLoad() runs once before the first update/render and onUnload() runs exactly
// once before the scene is destroyed.
class IScene {
public:
    virtual ~IScene() = default;

    virtual const char* name() const = 0;

    // Failure must throw with a descriptive message; the application reports
    // the error and shuts down instead of continuing with a broken scene.
    virtual void onLoad() = 0;
    virtual void onUnload() = 0;

    virtual void update(double deltaSeconds, const Input& input) = 0;
    virtual void render() = 0;
    virtual void renderUI() {}
};

}  // namespace vlt
