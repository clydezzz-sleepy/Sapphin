/***************************************************************************************************************************/
// _sapphin_engine.cpp
// This is the main file that will run the application.
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

int main() {
    bool continueRendering = true;

    // Main loop
    while (continueRendering) {
        // Welcome and instructions
        continueRendering = false;
        typewriterEffect("Welcome to Sapphin 3D Renderer.", CYAN, 50);
        typewriterEffect("The app where you can render your creations and show them to your friends.", CYAN, 50);
        typewriterEffect("If you don't have a file to display, you can render a default triangle.\nWrite 'triangle' without quotes.", BLUE, 30);
        typewriterEffect("Enter the name of the file to load it (without the .obj extension):", GREEN, 30);

        // Get filename from user
        std::string filename;
        std::getline(std::cin, filename);
        filename += ".obj";

        // Validate filename input
        while (filename == ".obj") {
            typewriterEffect("Please input a filename to simulate a .obj file.", BLUE, 30);
            typewriterEffect("Enter the name of the file to load it (without the .obj extension):", GREEN, 30);
            filename.clear();
            std::getline(std::cin, filename);
            if (filename.substr(filename.length() - 4) != ".obj") {
                typewriterEffect("Please pick a valid file format (.obj).", RED, 50);
            }
            filename += ".obj";
        }

        // Load model vertices
        std::vector<Vertex> vertices;
        if (fileExists(filename)) {
            typewriterEffect("Loading model from " + filename + "...", BLUE, 30);
            vertices = loadModel(filename);
        }
        else if (filename == "triangle.obj") {
            typewriterEffect("Loading default triangle...", RED, 30);
            // Default triangle vertices
            vertices = {
                Vertex{-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f},
                Vertex{ 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f},
                Vertex{ 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 1.0f}
            };
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            glDisable(GL_CULL_FACE);

        }
        else {
            typewriterEffect("File not found. Falling back to default triangle.\n(Make sure your input doesn't have any spaces if your file doesn't have any either.)", RED, 30);
            vertices = {
                Vertex{-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f},
                Vertex{ 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f},
                Vertex{ 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.5f, 1.0f}
            };
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            glDisable(GL_CULL_FACE);

        }

        // Initialize GLFW and create window
        GLFWwindow* window = initOpenGL();
        glfwWindowHint(GLFW_SAMPLES, 4);  // 4x MSAA
        glEnable(GL_MULTISAMPLE);

        if (!window) {
            std::cerr << "Failed to initialize OpenGL!" << std::endl;
            return -1;
        }

        // Add this check
        std::cout << "OpenGL Context Created Successfully" << std::endl;
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

        // Store the camera in the window user pointer (for callbacks)
        Camera camera(glm::vec3(0.0f, 0.0f, 3.0f)); // new Camera()
        glfwSetWindowUserPointer(window, &camera);

        // Prepare shaders
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        std::string vertexShaderSource = getDefaultVertexShader();
        std::string fragmentShaderSource = getDefaultFragmentShader();
        const char* vShaderCode = vertexShaderSource.c_str();
        const char* fShaderCode = fragmentShaderSource.c_str();
        glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
        glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);

        // Debug print shader sources
        std::cout << "Vertex Shader Source:\n" << vertexShaderSource << std::endl;
        std::cout << "Fragment Shader Source:\n" << fragmentShaderSource << std::endl;

        // Create shader program with the prepared sources
        GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

        // Add error checking
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL error after shader program creation: " << err << std::endl;
        }

        // Add debug check for color attribute
        GLint colorAttribLocation = glGetAttribLocation(shaderProgram, "aColor");
        std::cout << "Color attribute location: " << colorAttribLocation << std::endl;

        // Create and bind VAO and VBO
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Set up vertex attributes
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
        glEnableVertexAttribArray(0);

        // Normal attribute (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
        glEnableVertexAttribArray(1);

        // Texture coordinate attribute (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
        glEnableVertexAttribArray(2);

        // Color attribute (location = 3)
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
        glEnableVertexAttribArray(3);

        // Set up callbacks
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetCursorPosCallback(window, mouse_callback);

        // Print control instructions
        typewriterEffect("Controls:\n"
            "W/A/S/D: Move camera\n"
            "Mouse: Look around\n"
            "Scroll/Arrow keys: Zoom\n"
            "Left Shift: Speed up\n"
            "ESC: Exit\n"
            "N: Choose another file\n", CYAN, 30);

        // Timing variables
        float lastFrame = 0.0f;

        // Render loop
        while (!glfwWindowShouldClose(window)) {
            // Calculate delta time
            float currentFrame = glfwGetTime();
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Process input
            processInput(window, camera, deltaTime);

            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

            // Prepare matrices
            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 projection;
            updateCameraProjection(projection, camera);

            // Use shader program
            glUseProgram(shaderProgram);

            // Ensure we're rendering filled triangles, not wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // Model matrix (identity for now)
            glm::mat4 model = glm::mat4(1.0f);

            // Get uniform locations
            GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
            GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
            GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

            // Set uniforms
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

            // Draw the model
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);

        // Check if restart was requested
        void* userPtr = glfwGetWindowUserPointer(window);
        if (userPtr == reinterpret_cast<void*>(1)) {
            continueRendering = true;
        }
        else {
            // Just in case we need to check for keyboard input one last time
            if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
                continueRendering = true;
            }
        }
        
        // Delete new Camera()
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        delete cam;

        // Clean up GLFW
        glfwTerminate();

        if (continueRendering) {
            typewriterEffect("Restarting application...", GREEN, 30);
        }
        else {
            typewriterEffect("Exiting application...", GREEN, 30);
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    return 0;
}
