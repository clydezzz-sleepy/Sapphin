// _sapphin_render.cpp
// This is the file that renders everything.
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

// This file has all the mesh functions (for vertices, faces) set up.

GLFWwindow* initOpenGL() {
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW." << std::endl;
        return nullptr;
	}

	// Print GLFW version
	int major, minor, rev;
	glfwGetVersion(&major, &minor, &rev);
	std::cout << "GLFW Version: " << major << "." << minor << "." << rev << std::endl;

	// Set OpenGL version hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Sapphin 3D Renderer", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Print OpenGL version and vendor info
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// Set viewport
	glViewport(0, 0, 800, 600);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Enable backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    glFrontFace(GL_CW);
	glFrontFace(GL_CCW);

	// Set up error callback
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error " << error << ": " << description << std::endl;
		});

	// Set up window resize callback
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		});

	return window;
}

// Function to load shader source from file
std::string loadShaderSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to compile individual shader
GLuint compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed ("
            << (shaderType == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
            << "):\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}


// Function to create complete shader program
GLuint createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "Empty shader source" << std::endl;
        return 0;
    }

    // Compile shaders
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    if (!vertexShader || !fragmentShader) {
        if (vertexShader) glDeleteShader(vertexShader);
        if (fragmentShader) glDeleteShader(fragmentShader);
        return 0;
    }

    // Create and link program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check linking status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Add debug information
    GLint numAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
    std::cout << "Maximum number of vertex attributes supported: " << numAttributes << std::endl;

    // Verify program attributes
    GLint activeAttributes;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
    std::cout << "Number of active attributes: " << activeAttributes << std::endl;

    for (GLint i = 0; i < activeAttributes; i++) {
        GLchar name[32];
        GLint size;
        GLenum type;
        glGetActiveAttrib(program, i, sizeof(name), nullptr, &size, &type, name);
        std::cout << "Attribute " << i << ": " << name << " (type: " << type << ")" << std::endl;
    }

    return program;
}

GLuint createShaderProgram() {
    // Load shader sources
    std::string vertexSource = loadShaderSource("shaders/vertex_shader.glsl");
    std::string fragmentSource = loadShaderSource("shaders/fragment_shader.glsl");

    if (vertexSource.empty()) {
        std::cerr << "Failed to load vertex shader from file, using default." << std::endl;
        vertexSource = getDefaultVertexShader();
    }

    if (fragmentSource.empty()) {
        std::cerr << "Failed to load fragment shader from file, using default." << std::endl;
        fragmentSource = getDefaultFragmentShader();
    }

    // Use the version that takes string parameters
    return createShaderProgram(vertexSource, fragmentSource);
}

std::string getDefaultVertexShader() {
    return
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec3 aNormal;\n"
        "layout(location = 2) in vec2 aTexCoord;\n"
        "layout(location = 3) in vec4 aColor;\n"
        "\n"
        "out vec3 Normal;\n"
        "out vec2 TexCoord;\n"
        "out vec4 Color;\n"
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "    TexCoord = aTexCoord;\n"
        "    Color = aColor;\n"
        "}";
}

std::string getDefaultFragmentShader() {
    return
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 Normal;\n"
        "in vec4 Color;\n"
        "\n"
        "void main() {\n"
        "    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));\n"
        "    float diff = max(dot(normalize(Normal), lightDir), 0.0);\n"
        "    vec3 diffuse = vec3(0.7) * diff;\n"
        "    vec3 ambient = vec3(0.3);\n"
        "    FragColor = vec4((ambient + diffuse) * Color.rgb, Color.a);\n"
        "}";
}
