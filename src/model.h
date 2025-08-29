#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <glm/glm.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "texture.h"
#include "camera.h"

class Model {
public:
    Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
    VertexArrayObject vao;
    void draw(Shader& shader, Camera& camera);

private:
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
};

#endif // MODEL_H