// _sapphin_camera.h
// This header file includes all functionality for the camera.
// Sapphin 3D Renderer (OpenGL, GLFW/GLEW)

#pragma once  // Prevents multiple inclusions

// Headers
#include "headers/_sapphin_utils.h"
#include "headers/_sapphin_render.h"
#include "headers/_sapphin_modeling.h"
#include "headers/_sapphin_types.h"
#include "lib/GLEW.win32/GLEW-lib/include/GL/glew.h"
#include "lib/GLFW.win32/GLFW-lib/include/GLFW/glfw3.h"

// HPP files
#include "lib/GLM.win32/GLM-lib/glm/glm.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/matrix_transform.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/type_ptr.hpp"

// Enum for camera movement directions
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float NORMAL_SPEED = 6.75f;
    float SPEEDUP_MULTIPLIER = 2.0f;
    float mouseSensitivity;
    float zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);

    glm::mat4 getViewMatrix();
    void processKeyboardMovement(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    void smoothMoveToTarget(const glm::vec3& target, float smoothFactor);

private:
    void updateCameraVectors();
};

// Callback function declarations
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Additional function declarations
void SetupMouseCapture(GLFWwindow* window);
void processInput(GLFWwindow* window, Camera& camera, float deltaTime);
void updateCameraProjection(glm::mat4& projection, const Camera& camera);
