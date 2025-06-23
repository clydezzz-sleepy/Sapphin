// _sapphin_render.h
// This file includes all functionals for rendering inside of the application.
// Sapphin 3D Renderer ((OpenGL, GLFW/GLEW))
#pragma once  // Prevents multiple inclusions

// Headers
#include <string>
#include <vector>
#include "headers/_sapphin_utils.h"
#include "headers/_sapphin_modeling.h"
#include "headers/_sapphin_types.h"
#include "lib/GLEW.win32/GLEW-lib/include/GL/glew.h"
#include "lib/GLFW.win32/GLFW-lib/include/GLFW/glfw3.h"

// HPP files
#include "lib/GLM.win32/GLM-lib/glm/glm.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/matrix_transform.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/type_ptr.hpp"

GLFWwindow* initOpenGL();
GLuint createShaderProgram();
GLuint createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);
void renderModel(GLFWwindow* window, const std::vector<Vertex>& vertices, GLuint shaderProgram);

// Shader source generators
std::string getDefaultVertexShader();
std::string getDefaultFragmentShader();
