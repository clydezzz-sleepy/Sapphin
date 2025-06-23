// _sapphin_camera.cpp
// This controls the camera.
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

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float cameraZoom = 45.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Modify zoom limits
const float MIN_ZOOM = 0.1f;    // Smaller number = closer zoom
const float MAX_ZOOM = 179.0f;  // Larger number = wider FOV
const float ZOOM_SPEED = 2.0f;  // Adjust for faster/slower zoom

// Constructor
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(NORMAL_SPEED),
    mouseSensitivity(0.2f), zoom(45.0f) {
    this->position = position;
    this->worldUp = up;
    this->yaw = yaw;
    this->pitch = pitch;
    updateCameraVectors();
}

// Get view matrix
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

// Process keyboard movement
void Camera::processKeyboardMovement(Camera_Movement direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (direction == FORWARD) position += front * velocity;
    if (direction == BACKWARD) position -= front * velocity;
    if (direction == LEFT) position -= right * velocity;
    if (direction == RIGHT) position += right * velocity;
    if (direction == UP) position += up * velocity;
    if (direction == DOWN) position -= up * velocity;
    updateCameraVectors();
}

// Process mouse movement
void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
    updateCameraVectors();
}

// Process scroll
void Camera::processMouseScroll(float yoffset) {
    zoom -= yoffset;
    if (zoom < 1.0f) zoom = -45.0f;
    if (zoom > 45.0f) zoom = 45.0f;
}

// Smooth move to target
void Camera::smoothMoveToTarget(const glm::vec3& target, float smoothFactor) {
    position = glm::mix(position, target, smoothFactor);
    updateCameraVectors();
}

// Update camera vectors
void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

// Scroll callback
void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
	static bool firstMouse = true;
	static double lastX = 0.0;
	static double lastY = 0.0;

	Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (!camera) return;
	camera->processMouseScroll(static_cast<float>(ypos));
	
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		return;
	}
	float xoffset = static_cast<float>(xpos - lastX);
	float yoffset = static_cast<float>(lastY - ypos);  // Reversed since y-coordinates range from bottom to top

	lastX = xpos;
	lastY = ypos;

	// Process mouse movement
	camera->processMouseMovement(xoffset, yoffset);
}

// Set up mouse capture
void SetupMouseCapture(GLFWwindow* window) {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	Camera* camera = new Camera();
	glfwSetWindowUserPointer(window, camera);
	glfwSetCursorPosCallback(window, mouse_callback);
}

// Mouse callback function
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static bool firstMouse = true;
	static float lastX = SCR_WIDTH / 2.0f;
	static float lastY = SCR_HEIGHT / 2.0f;

	Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		return;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (camera) {
		camera->processMouseMovement(xoffset, yoffset);
	}
}

// Process all input with this function
void processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(1));
        glfwSetWindowShouldClose(window, true);
    }

    // Sprint handling
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.movementSpeed = camera.NORMAL_SPEED * camera.SPEEDUP_MULTIPLIER;
    }
    else {
        camera.movementSpeed = camera.NORMAL_SPEED;
    }

    // Keyboard camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardMovement(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardMovement(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardMovement(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardMovement(RIGHT, deltaTime);

    // Keyboard zoom
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.processMouseScroll(1.0f);  // Zoom in
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.processMouseScroll(-1.0f); // Zoom out
}

// Update the projection matrix
void updateCameraProjection(glm::mat4& projection, const Camera& camera) {
	projection = glm::perspective(glm::radians(camera.zoom),
		(float)SCR_WIDTH / (float)SCR_HEIGHT,
		0.1f, 500.0f);
}
