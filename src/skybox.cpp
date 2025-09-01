#include "skybox.h"
#include "stb_image.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// Skybox vertices (positions only - no normals or UVs needed)
static float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

Skybox::Skybox(const std::vector<std::string>& faces) 
    : VAO(0), VBO(0), textureID(0), initialized(false) {
    
    if (faces.size() != 6) {
        std::cerr << "Error: Skybox requires exactly 6 texture faces!" << std::endl;
        return;
    }
    
    setupCube();
    loadCubemapTextures(faces);
    initialized = true;
}

Skybox::~Skybox() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (textureID != 0) glDeleteTextures(1, &textureID);
}

void Skybox::setupCube() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Skybox::loadCubemapTextures(const std::vector<std::string>& faces) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Face names for debugging
    std::vector<std::string> faceNames = {"Right (+X)", "Left (-X)", "Top (+Y)", "Bottom (-Y)", "Front (+Z)", "Back (-Z)"};

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;
            
            std::cout << "Loading " << faceNames[i] << ": " << faces[i] << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                        0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "ERROR: Cubemap texture failed to load at path: " << faces[i] << std::endl;
            std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
            
            // Create a fallback colored texture for missing faces
            unsigned char fallbackColor[3];
            switch(i) {
                case 0: fallbackColor[0] = 255; fallbackColor[1] = 0; fallbackColor[2] = 0; break; // Red for +X
                case 1: fallbackColor[0] = 0; fallbackColor[1] = 255; fallbackColor[2] = 0; break; // Green for -X
                case 2: fallbackColor[0] = 0; fallbackColor[1] = 0; fallbackColor[2] = 255; break; // Blue for +Y
                case 3: fallbackColor[0] = 255; fallbackColor[1] = 255; fallbackColor[2] = 0; break; // Yellow for -Y
                case 4: fallbackColor[0] = 255; fallbackColor[1] = 0; fallbackColor[2] = 255; break; // Magenta for +Z
                case 5: fallbackColor[0] = 0; fallbackColor[1] = 255; fallbackColor[2] = 255; break; // Cyan for -Z
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, fallbackColor);
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::draw(Shader& shader, Camera& camera) {
    if (!initialized) return;
    
    // Change depth function so depth test passes when values are equal to depth buffer's content
    glDepthFunc(GL_LEQUAL);
    
    shader.activate();
    
    // Remove translation from view matrix - we want the skybox to move with the camera
    glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));
    glm::mat4 projection = camera.getProjectionMatrix();
    
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));
    
    // Skybox cube
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glUniform1i(glGetUniformLocation(shader.ID, "skybox"), 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // Set depth function back to default
    glDepthFunc(GL_LESS);
    
    shader.deactivate();
}

void Skybox::bind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void Skybox::unbind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}