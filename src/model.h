#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <glm/glm.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "texture.h"
#include "camera.h"
#include "shader.h"

class Model {
public:
    Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& colors, const std::vector<Texture>& textures, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs);
    void draw(Shader& shader, Camera& camera);

    VertexArrayObject vao;
    VertexBufferObject vbo;
    ElementBufferObject ebo;

private:
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> colors;
};

#endif // MODEL_H