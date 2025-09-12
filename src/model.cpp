#include "model.h"
#include <iostream>

static std::vector<Vertex> assembleVertices(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& colors,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& uvs,
    const std::vector<glm::vec3>& tangents,
    const std::vector<glm::vec3>& bitangents
) {
    std::vector<Vertex> assembled;
    assembled.reserve(positions.size());
    for (size_t i = 0; i < positions.size(); ++i) {
        Vertex vert;
        vert.position = positions[i];
        vert.color = (i < colors.size()) ? colors[i] : glm::vec3(1.0f);
        vert.normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f, 0.0f, 1.0f);
        vert.uv = (i < uvs.size()) ? uvs[i] : glm::vec2(0.0f);
        vert.tangent = (i < tangents.size()) ? tangents[i] : glm::vec3(0.0f);
        vert.bitangent = (i < bitangents.size()) ? bitangents[i] : glm::vec3(0.0f);
        assembled.push_back(vert);
    }
    return assembled;
}

Model::Model(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, 
             const std::vector<glm::vec3>& colors, std::vector<Texture>& textures, 
             const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs,
             const std::vector<glm::vec3>& tangents, const std::vector<glm::vec3>& bitangents,
             const glm::mat4x4& modelMatrix, const MaterialProperties& material) :
    vertices(vertices), indices(indices), colors(colors), textures(std::move(textures)), 
    normals(normals), uvs(uvs), tangents(tangents), bitangents(bitangents), initialized(false), modelMatrix(modelMatrix), material(material)
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
    std::vector<Vertex> assembledVertices = assembleVertices(vertices, colors, normals, uvs, tangents, bitangents);
    //calculateTangents(assembledVertices, indices);

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
    vao->linkAttrib(*vbo, 4, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    vao->linkAttrib(*vbo, 5, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
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
}


void Model::calculateBitangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    // Calculates bitangents for each vertex based on tangent and normal
    for (auto& vertex : vertices) {
        // Ensure tangent and normal are normalized
        glm::vec3 tangent = glm::normalize(vertex.tangent);
        glm::vec3 normal = glm::normalize(vertex.normal);

        // Bitangent is cross product of normal and tangent
        vertex.bitangent = glm::normalize(glm::cross(normal, tangent));
    }
}

void Model::calculateTangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    // Initialize tangents and bitangents to zero
    for (auto& vertex : vertices) {
        vertex.tangent = glm::vec3(0.0f);
        vertex.bitangent = glm::vec3(0.0f);
    }
    
    // Calculate tangents for each triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;
        
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];
        
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) continue;
        
        Vertex& v0 = vertices[i0];
        Vertex& v1 = vertices[i1];
        Vertex& v2 = vertices[i2];
        
        // Calculate triangle edges
        glm::vec3 deltaPos1 = v1.position - v0.position;
        glm::vec3 deltaPos2 = v2.position - v0.position;
        
        // Calculate UV deltas
        glm::vec2 deltaUV1 = v1.uv - v0.uv;
        glm::vec2 deltaUV2 = v2.uv - v0.uv;
        
        // Calculate tangent and bitangent
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        
        if (!std::isfinite(r)) {
            // Fallback if UVs are degenerate
            continue;
        }
        
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
        
        // Add to vertices (we'll average later)
        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
        
        v0.bitangent += bitangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
    }
    
    // Normalize and orthogonalize tangents using Gram-Schmidt process
    for (auto& vertex : vertices) {
        if (length(vertex.tangent) > 0.0f) {
            // Orthogonalize tangent with respect to normal
            vertex.tangent = normalize(vertex.tangent - dot(vertex.tangent, vertex.normal) * vertex.normal);
        } else {
            // Fallback tangent if calculation failed
            glm::vec3 c1 = cross(vertex.normal, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::vec3 c2 = cross(vertex.normal, glm::vec3(0.0f, 1.0f, 0.0f));
            vertex.tangent = normalize(length(c1) > length(c2) ? c1 : c2);
        }
        
        if (length(vertex.bitangent) > 0.0f) {
            vertex.bitangent = normalize(vertex.bitangent);
        } else {
            // Calculate bitangent from normal and tangent
            vertex.bitangent = normalize(cross(vertex.normal, vertex.tangent));
        }
    }
}

void Model::drawShadow(Shader& shadowShader) {
    // Initialize OpenGL objects on first draw
    if (!initialized) {
        initializeGL();
        if (!initialized) {
            std::cerr << "Failed to initialize OpenGL objects for model!" << std::endl;
            return;
        }
    }

    // Only set the model matrix for shadow rendering
    shadowShader.setMat4("model", glm::value_ptr(this->getModelMatrix()));

    // Don't bind textures or set material properties for shadow pass
    
    // Draw only geometry
    vao->bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    vao->unbind();
}

void Model::drawGeometryOnly() {
    // Initialize OpenGL objects on first draw
    if (!initialized) {
        initializeGL();
        if (!initialized) {
            std::cerr << "Failed to initialize OpenGL objects for model!" << std::endl;
            return;
        }
    }

    // This method ONLY draws geometry - no texture binding, no uniform setting
    // Assumes shader is already active and model matrix is already set
    
    // Handle face culling
    if (material.doubleSided) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    
    // Draw geometry
    vao->bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    vao->unbind();
}
