#include "model.h"
#include <iostream>

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

Model::Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, 
             const std::vector<glm::vec3>& colors, std::vector<Texture>& textures, 
             const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs,
             const glm::mat4x4& modelMatrix, const MaterialProperties& material) :
    vertices(vertices), indices(indices), colors(colors), textures(std::move(textures)), 
    normals(normals), uvs(uvs), initialized(false), modelMatrix(modelMatrix), material(material)
{
    // Don't create OpenGL objects in constructor - defer until first draw
    //std::cout << "Model constructor called - deferring OpenGL object creation" << std::endl;
}

void Model::initializeGL() {
    if (initialized) return;
    
    //std::cout << "Initializing OpenGL objects for model..." << std::endl;
    
    // Check OpenGL context
    if (!glfwGetCurrentContext()) {
        std::cerr << "Error: No OpenGL context when initializing model!" << std::endl;
        return;
    }
    
    // Assemble vertices
    std::vector<Vertex> assembledVertices = assembleVertices(vertices, colors, normals, uvs);
    
    // Create OpenGL objects in the correct order
    vao = std::make_unique<VertexArrayObject>();
    vbo = std::make_unique<VertexBufferObject>(assembledVertices);
    ebo = std::make_unique<ElementBufferObject>(indices);
    
    // Setup vertex attributes
    vao->bind();
    vbo->bind();
    ebo->bind();
    
    // Link attributes
    vao->linkAttrib(*vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    vao->linkAttrib(*vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    vao->linkAttrib(*vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    vao->linkAttrib(*vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    
    vao->unbind();
    vbo->unbind();
    ebo->unbind();
    
    initialized = true;
    //std::cout << "Model OpenGL objects initialized successfully" << std::endl;
}

void Model::draw(Shader& shader, Camera& camera) {
    // Initialize OpenGL objects on first draw
    if (!initialized) {
        initializeGL();
        if (!initialized) {
            std::cerr << "Failed to initialize OpenGL objects for model!" << std::endl;
            return;
        }
    }
    shader.activate();

    //std::cout << "Drawing Model with VAO ID: " << vao->getID() << std::endl;
    
    // Bind textures
    for (size_t i = 0; i < textures.size(); ++i) {
        std::string uniformName;
        switch (textures[i].type) {
            case TextureType::Diffuse:  
                uniformName = "baseColorTexture"; 
                break;
            case TextureType::Normal:   
                uniformName = "normalTexture"; 
                break;
            case TextureType::Metallic: 
                uniformName = "metallicRoughnessTexture"; 
                break;
            case TextureType::Roughness: 
                uniformName = "metallicRoughnessTexture"; 
                break;
            case TextureType::Occlusion: 
                uniformName = "occlusionTexture"; 
                break;
            case TextureType::Emissive: 
                uniformName = "emissiveTexture"; 
                break;
            case TextureType::Specular: 
                uniformName = "specularTexture"; 
                break;
            default: 
                uniformName = "texture" + std::to_string(i); 
                break;
        }
        textures[i].bind();
        textures[i].texUnit(shader, uniformName.c_str(), textures[i].unit);
    }


    // Set matrices
    //std::cout << camera.printViewMatrix() << std::endl;
    shader.setMat4("view", glm::value_ptr(camera.getViewMatrix()));
    shader.setMat4("projection", glm::value_ptr(camera.getProjectionMatrix()));
    shader.setMat4("model", glm::value_ptr(this->getModelMatrix()));


    shader.setVec4("baseColorFactor", glm::value_ptr(material.baseColorFactor));
    shader.setFloat("alphaCutoff", material.alphaCutoff);
    shader.setFloat("metallicFactor", material.metallicFactor);
    shader.setFloat("roughnessFactor", material.roughnessFactor);
    shader.setBool("useAlphaBlending", material.alphaMode_MASK);
    // Enable/disable face culling based on doubleSided
    
    if (material.doubleSided) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
    }
    // Draw
    vao->bind();

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    
    // Check for errors after drawing
    // GLenum err = glGetError();
    // if (err != GL_NO_ERROR) {
    //     std::cerr << "OpenGL error after draw: " << err << std::endl;
    // }

    // Cleanup
    for (auto& texture : textures) {
        texture.unbind();
    }
    
    vao->unbind();
    shader.deactivate();
}