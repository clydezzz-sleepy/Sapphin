// _sapphin_utils.cpp
// This is for extra functionality (possibly also needed).
// Sapphin 3D Renderer ((OpenGL, GLFW/GLEW))
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

// Headers
#include "headers/_sapphin_utils.h"
#include "headers/_sapphin_camera.h"
#include "headers/_sapphin_render.h"
#include "headers/_sapphin_modeling.h"
#include "headers/_sapphin_types.h"
#include "lib/GLEW.win32/GLEW-lib/include/GL/glew.h"
#include "lib/GLFW.win32/GLFW-lib/include/GLFW/glfw3.h"

// HPP files
#include "lib/GLM.win32/GLM-lib/glm/glm.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/matrix_transform.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/type_ptr.hpp"

// Set a function for a typewriter effect for text
void typewriterEffect(const std::string& text, const std::string& color, int milliseconds_delay) {
    std::cout << color;  // Set color
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds_delay));
    }
    std::cout << RESET << std::endl; // Reset color
}

// First, set fileExists() for checking the filename
bool fileExists(const std::string& filename) {
	std::ifstream file(filename);
	return file.good();
}

void checkGLError(const char* operation) {
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		std::string errorMsg = "OpenGL error after " + std::string(operation) +
			": " + std::to_string(error);
		typewriterEffect(errorMsg, RED, 30);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
