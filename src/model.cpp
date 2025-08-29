#include "model.h"

Model::Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures) :
    vertices(vertices), indices(indices), textures(textures)
{
    VertexArrayObject vao;
    VertexBufferObject vbo(vertices);
    ElementBufferObject ebo(indices);
    vao.bind();
    vbo.bind();
    ebo.bind();
    for (const auto& texture : textures) {
        texture.bind();
    }
}