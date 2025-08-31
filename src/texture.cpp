#include "texture.h"
#include <iostream>
#include <filesystem>

Texture::Texture(const char* image, TextureType texType, GLuint slot)
    : type(texType), unit(slot), loaded(false), filePath(image), initialized(false), ID(0)
{
    // Don't create OpenGL objects here - defer until first use
}

Texture::Texture(Texture&& other) noexcept
    : ID(other.ID), type(other.type), unit(other.unit), loaded(other.loaded), 
      filePath(std::move(other.filePath)), initialized(other.initialized)
{
    other.ID = 0;
    other.loaded = false;
    other.initialized = false;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        // Clean up current texture
        if (ID != 0) {
            glDeleteTextures(1, &ID);
        }
        
        // Move from other
        ID = other.ID;
        type = other.type;
        unit = other.unit;
        loaded = other.loaded;
        filePath = std::move(other.filePath);
        initialized = other.initialized;
        
        // Reset other
        other.ID = 0;
        other.loaded = false;
        other.initialized = false;
    }
    return *this;
}

void Texture::initializeGL() {
    if (initialized) return;

    checkGLError("init texture");

    // Check if file exists
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Texture file does not exist: " << filePath << std::endl;
        return;
    }
    
    // Generate texture
    glGenTextures(1, &ID);
    checkGLError("generating textures");
    if (ID == 0) {
        std::cerr << "Failed to generate texture ID" << std::endl;
        return;
    }
    
    
    glBindTexture(GL_TEXTURE_2D, ID);
    checkGLError("binding texture");

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    checkGLError("setting texture parameters");

    // Load image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        
        // Determine format
        GLenum format = GL_RGB;
        GLenum internalFormat = GL_RGB8;
        
        switch (nrChannels) {
            case 1:
                format = GL_RED;
                internalFormat = GL_R8;
                break;
            case 3:
                format = GL_RGB;
                internalFormat = GL_RGB8;
                break;
            case 4:
                format = GL_RGBA;
                internalFormat = GL_RGBA8;
                break;
            default:
                std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
                stbi_image_free(data);
                return;
        }
        
        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        checkGLError("uploading texture data");
        glGenerateMipmap(GL_TEXTURE_2D);
        checkGLError("generating mipmaps");

        // Verify texture was created successfully
        GLint textureWidth;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
        if (textureWidth > 0) {
            loaded = true;
        } else {
            std::cerr << "Texture upload failed - width is 0" << std::endl;
        }
    } else {
        std::cerr << "Failed to load texture data from: " << filePath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
    }
    
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGLError("unbinding texture");
    initialized = true;
}

void Texture::loadTexture() {
    if (!initialized) {
        initializeGL();
    }
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit) {
    // Ensure texture is loaded
    if (!initialized) {
        loadTexture();
    }
    
    // Check if uniform exists
    GLint uniformLocation = glGetUniformLocation(shader.ID, uniform);
    if (uniformLocation == -1) {
        std::cerr << "Warning: Uniform '" << uniform << "' not found in shader" << std::endl;
        return;
    }

    glUniform1i(uniformLocation, static_cast<GLint>(unit));
    checkGLError("setting texture unit");
}

void Texture::bind() {
    // Ensure texture is loaded
    if (!initialized) {
        loadTexture();
    }
    
    if (!loaded || ID == 0) {
        std::cerr << "Warning: Attempting to bind invalid texture (ID: " << ID << ", loaded: " << loaded << ")" << std::endl;
        return;
    }
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
    checkGLError("binding texture");
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGLError("unbinding texture");
}

Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}