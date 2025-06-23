// _sapphin_modeling.cpp
// This will render the actual file that contains the 3D model and display it.
// Sapphin 3D Renderer ((OpenGL, GLFW/GLEW))

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

// Headers
#include "headers/_sapphin_utils.h"
#include "headers/_sapphin_camera.h"
#include "headers/_sapphin_render.h"
#include "headers/_sapphin_modeling.h"
#include "lib/GLEW.win32/GLEW-lib/include/GL/glew.h"
#include "lib/GLFW.win32/GLFW-lib/include/GLFW/glfw3.h"

// HPP files
#include "lib/GLM.win32/GLM-lib/glm/glm.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/matrix_transform.hpp"
#include "lib/GLM.win32/GLM-lib/glm/gtc/type_ptr.hpp"

void parseIndices(const std::string& str, int& v, int& t, int& n) {
    // Initialize indices to -1 (indicating not present)
    v = t = n = -1;

    // Find positions of '/' characters
    size_t first_slash = str.find('/');

    if (first_slash == std::string::npos) {
        // If no slashes, just vertex index
        v = std::stoi(str) - 1;
        return;
    }

    // Get vertex index (always present)
    v = std::stoi(str.substr(0, first_slash)) - 1;

    size_t second_slash = str.find('/', first_slash + 1);

    if (second_slash == std::string::npos) {
        // If only one slash, we have vertex/texture
        if (str.length() > first_slash + 1) {
            t = std::stoi(str.substr(first_slash + 1)) - 1;
        }
        return;
    }

    // We have vertex/texture/normal
    // Check for texture coordinate
    if (second_slash > first_slash + 1) {
        // There's something between the slashes
        t = std::stoi(str.substr(first_slash + 1, second_slash - first_slash - 1)) - 1;
    }

    // Check for normal
    if (str.length() > second_slash + 1) {
        n = std::stoi(str.substr(second_slash + 1)) - 1;
    }
}

std::vector<Vertex> loadModel(const std::string& filename) {
    std::vector<Vertex> vertices;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
        return vertices;
    }

    // Data containers
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> fileNormals;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec4> colors;
    
    // Temporary face storage
    struct FaceData { 
        int posIndices[3]; 
        int texIndices[3]; 
        int normIndices[3]; 
    };
    std::vector<FaceData> faces;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            // Vertex position
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            
            // Vertex color
            glm::vec4 color(0.7f, 0.7f, 0.7f, 1.0f);
            float r, g, b;
            if (iss >> r >> g >> b) {
                color.r = r;
                color.g = g;
                color.b = b;
                float a;
                if (iss >> a) color.a = a;
            }
            positions.push_back(pos);
            colors.push_back(color);
        }
        else if (type == "vn") {
            // Vertex normal (from file)
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            fileNormals.push_back(normal);
        }
        else if (type == "vt") {
            // Texture coordinate
            glm::vec2 tex;
            iss >> tex.x >> tex.y;
            texcoords.push_back(tex);
        }
        else if (type == "f") {
            // Face definition
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;
            
            FaceData face;
            parseIndices(v1, face.posIndices[0], face.texIndices[0], face.normIndices[0]);
            parseIndices(v2, face.posIndices[1], face.texIndices[1], face.normIndices[1]);
            parseIndices(v3, face.posIndices[2], face.texIndices[2], face.normIndices[2]);
            
            faces.push_back(face);
        }
    }

    // Compute vertex normals through averaging
    std::vector<glm::vec3> vertexNormals(positions.size(), glm::vec3(0.0f));
    if (!faces.empty()) {
        for (const auto& face : faces) {
            glm::vec3 v1 = positions[face.posIndices[0]];
            glm::vec3 v2 = positions[face.posIndices[1]];
            glm::vec3 v3 = positions[face.posIndices[2]];
            
            glm::vec3 edge1 = v2 - v1;
            glm::vec3 edge2 = v3 - v1;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
            
            vertexNormals[face.posIndices[0]] += faceNormal;
            vertexNormals[face.posIndices[1]] += faceNormal;
            vertexNormals[face.posIndices[2]] += faceNormal;
        }
        
        for (auto& normal : vertexNormals) {
            if (glm::length(normal) > 0.0f) {
                normal = glm::normalize(normal);
            }
        }
    }

    // Create vertices using computed normals
    for (const auto& face : faces) {
        for (int i = 0; i < 3; i++) {
            Vertex vertex;
            int posIdx = face.posIndices[i];
            
            // Position and color
            vertex.x = positions[posIdx].x;
            vertex.y = positions[posIdx].y;
            vertex.z = positions[posIdx].z;
            vertex.r = colors[posIdx].r;
            vertex.g = colors[posIdx].g;
            vertex.b = colors[posIdx].b;
            vertex.a = colors[posIdx].a;
            
            // Use computed normal
            vertex.nx = vertexNormals[posIdx].x;
            vertex.ny = vertexNormals[posIdx].y;
            vertex.nz = vertexNormals[posIdx].z;
            
            // Texture coordinates
            if (face.texIndices[i] >= 0 && face.texIndices[i] < texcoords.size()) {
                vertex.u = texcoords[face.texIndices[i]].x;
                vertex.v = texcoords[face.texIndices[i]].y;
            } else {
                vertex.u = 0.0f;
                vertex.v = 0.0f;
            }
            
            vertices.push_back(vertex);
        }
    }

    // Debug output
    std::cout << "Model loading statistics:" << std::endl;
    std::cout << "Vertices loaded: " << vertices.size() << std::endl;
    std::cout << "Normals computed: " << vertexNormals.size() << std::endl;
    std::cout << "UV coords loaded: " << texcoords.size() << std::endl;

    file.close();
    return vertices;
}

// Function to render the model (basic)
void renderModel(GLFWwindow* window, const std::vector<Vertex>& vertices, GLuint shaderProgram) {
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// Vertex attributes setup
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);  // Position

	// Add normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);  // Normal

	// Add texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);  // UV

	// Add color attribute
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);  // Color

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
