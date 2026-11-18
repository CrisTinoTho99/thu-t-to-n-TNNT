#pragma once

// Every translation unit that touches OpenGL includes this header instead of
// <GLFW/glfw3.h>, so GLAD declarations exist before any GL call. GLFW's own
// legacy GL include is disabled with GLFW_INCLUDE_NONE (also set in CMake).
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include <glad/gl.h>

#include <GLFW/glfw3.h>
