// _sapphin_utils.h
// This header file includes all extra functionals (that may be needed).
// Sapphire 3D Renderer ((OpenGL, GLFW/GLEW))
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
std::vector<Vertex> loadModel(const std::string& filename);
GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
void renderModel(GLFWwindow* window, const std::vector<Vertex>& vertices, GLuint shaderProgram);

// Shader source generators
std::string getDefaultVertexShader();
std::string getDefaultFragmentShader();

// Color definitions
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"

// Function declaration
void typewriterEffect(const std::string& text, const std::string& color = "", int milliseconds_delay = 50);
bool fileExists(const std::string& filename);
void checkGLError(const std::string& message);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void GetDefaultVertexShader();
void GetDefaultFragmentShader();
