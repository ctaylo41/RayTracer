#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "texture.h"
#include "camera.h"
#include "shader.h"
#include "GLFW/glfw3.h"


class Model {
public:
    Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, 
          const std::vector<glm::vec3>& colors, std::vector<Texture>& textures, 
          const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs);
    
    // Add move constructor and move assignment operator
    Model(Model&& other) noexcept = default;
    Model& operator=(Model&& other) noexcept = default;
    
    // Delete copy constructor and copy assignment operator
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    
    void draw(Shader& shader, Camera& camera);

private:
    // Use smart pointers to manage OpenGL objects
    std::unique_ptr<VertexArrayObject> vao;
    std::unique_ptr<VertexBufferObject> vbo;
    std::unique_ptr<ElementBufferObject> ebo;
    
    // Data storage
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> colors;
    
    bool initialized;
    
    void initializeGL();
};

#endif // MODEL_H