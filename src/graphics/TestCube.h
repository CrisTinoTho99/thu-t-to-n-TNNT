#pragma once

namespace vlt {

// Placeholder geometry for the platform milestone: a coloured unit cube with
// per-vertex colours. It proves the GLAD loader, shader pipeline, depth test and
// camera work before any map model exists. Not a map asset.
class TestCube {
public:
    TestCube();
    ~TestCube();

    TestCube(const TestCube&) = delete;
    TestCube& operator=(const TestCube&) = delete;

    void draw() const;

private:
    unsigned int m_vertexArray = 0;
    unsigned int m_vertexBuffer = 0;
    unsigned int m_indexBuffer = 0;
};

}  // namespace vlt
