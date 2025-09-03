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

struct MaterialProperties {
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    float alphaCutoff = 0.5f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    bool alphaMode_MASK = false;
    bool doubleSided = false;
};


class Model {
public:
    Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, 
          const std::vector<glm::vec3>& colors, std::vector<Texture>& textures, 
          const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<glm::vec3>& tangents,
          const std::vector<glm::vec3>& bitangents, const glm::mat4x4& modelMatrix, const MaterialProperties& material);
    
    // Add move constructor and move assignment operator
    Model(Model&& other) noexcept = default;
    Model& operator=(Model&& other) noexcept = default;
    
    // Delete copy constructor and copy assignment operator
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    glm::mat4x4 getModelMatrix() const { return modelMatrix; }
    void draw(Shader& shader, Camera& camera);
    MaterialProperties getMaterialProperties() const { return material; }
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
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    glm::mat4x4 modelMatrix;
    
    bool initialized;

    MaterialProperties material;
    void calculateTangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void calculateBitangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void initializeGL();
};

#endif // MODEL_H