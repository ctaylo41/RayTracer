#include "texture.h"
#include <iostream>
#include <filesystem>

Texture::Texture(const char* image, TextureType texType, GLuint slot)
    : type(texType), unit(slot), loaded(false)
{
    std::cout << "Loading texture: " << image << std::endl;
    
    // Check if file exists
    if (!std::filesystem::exists(image)) {
        std::cerr << "Texture file does not exist: " << image << std::endl;
        createFallbackTexture();
        return;
    }
    
    // Generate texture
    glGenTextures(1, &ID);
    if (ID == 0) {
        std::cerr << "Failed to generate texture ID" << std::endl;
        createFallbackTexture();
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, ID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Flip texture coordinates
    unsigned char* data = stbi_load(image, &width, &height, &nrChannels, 0);
    
    if (data) {
        std::cout << "Loaded texture: " << width << "x" << height << " with " << nrChannels << " channels" << std::endl;
        
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
                createFallbackTexture();
                return;
        }
        
        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
        // Check for errors after texture upload
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error during texture upload: " << error << std::endl;
            stbi_image_free(data);
            createFallbackTexture();
            return;
        }
        
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Verify texture was created successfully
        GLint textureWidth;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
        if (textureWidth > 0) {
            loaded = true;
            std::cout << "Texture loaded successfully with ID: " << ID << std::endl;
        } else {
            std::cerr << "Texture upload failed - width is 0" << std::endl;
            createFallbackTexture();
        }
    } else {
        std::cerr << "Failed to load texture data from: " << image << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        createFallbackTexture();
    }
    
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind
}

void Texture::createFallbackTexture() {
    std::cout << "Creating fallback texture..." << std::endl;
    
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    
    // Create a simple 2x2 checkerboard pattern
    unsigned char fallbackData[] = {
        255, 0, 255, 255,  // Magenta
        0, 0, 0, 255,      // Black
        0, 0, 0, 255,      // Black  
        255, 0, 255, 255   // Magenta
    };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallbackData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    loaded = true;
    std::cout << "Fallback texture created with ID: " << ID << std::endl;
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit) {
    // Check if uniform exists
    GLint uniformLocation = glGetUniformLocation(shader.ID, uniform);
    if (uniformLocation == -1) {
        std::cout << "Warning: Uniform '" << uniform << "' not found in shader" << std::endl;
        return;
    }
    
    glUniform1i(uniformLocation, static_cast<GLint>(unit));
}

void Texture::bind() {
    if (!loaded) {
        std::cerr << "Warning: Attempting to bind unloaded texture ID: " << ID << std::endl;
        return;
    }
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}