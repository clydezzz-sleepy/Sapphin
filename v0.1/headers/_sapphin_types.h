// _sapphin_types.h
// This file defines common types for the Sapphin 3D Renderer.
// Sapphin 3D Renderer ((OpenGL, GLFW/GLEW))

#pragma once  // Prevents multiple inclusions

// HPP files
#include "lib/GLM.win32/GLM-lib/glm/glm.hpp" // Just because Vertex only uses GLM.

// Vertex structure used for rendering
struct Vertex {
    float x, y, z;    // Position
    float nx, ny, nz; // Normal
    float u, v;       // Texture coordinates
    float r, g, b, a;
};
