#include "model.h"

static std::vector<Vertex> assembleVertices(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& colors,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& uvs
) {
    std::vector<Vertex> assembled;
    assembled.reserve(positions.size());
    for (size_t i = 0; i < positions.size(); ++i) {
        Vertex vert;
        vert.position = positions[i];
        vert.color = (i < colors.size()) ? colors[i] : glm::vec3(1.0f);
        vert.normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f, 0.0f, 1.0f);
        vert.uv = (i < uvs.size()) ? uvs[i] : glm::vec2(0.0f);
        assembled.push_back(vert);
    }
    return assembled;
}

Model::Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& colors, const std::vector<Texture>& textures, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs) :
    vertices(vertices), indices(indices), colors(colors), textures(textures), normals(normals), uvs(uvs), vao(), ebo(indices), vbo(assembleVertices(vertices, colors, normals, uvs))
{
    
}

void Model::draw(Shader& shader, Camera& camera) {
    vao.bind();
    vbo.bind();
    ebo.bind();

    vao.linkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    vao.linkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    vao.linkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    vao.linkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    shader.activate();
    // Bind all textures and set their uniforms
    for (size_t i = 0; i < textures.size(); ++i) {

        std::string uniformName;
        switch (textures[i].type) {
            case TextureType::Diffuse:  uniformName = "diffuseMap"; break;
            case TextureType::Specular: uniformName = "specularMap"; break;
            // Add other types as needed
            default: uniformName = "texture" + std::to_string(i); break;
        }
    
        textures[i].bind();
        textures[i].texUnit(shader, uniformName.c_str(), static_cast<GLuint>(i));
        // Texture parameters should be set during texture creation, not every draw call
    }

    // Unbind textures after drawing
    for (size_t i = 0; i < textures.size(); ++i) {
        textures[i].unbind();
    }

    shader.setMat4("view", glm::value_ptr(camera.getViewMatrix()));
    shader.setMat4("projection", glm::value_ptr(camera.getProjectionMatrix()));
    shader.setMat4("model", glm::value_ptr(camera.getModelMatrix()));

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    vao.unbind();
    vbo.unbind();
    ebo.unbind();
    shader.deactivate();
}