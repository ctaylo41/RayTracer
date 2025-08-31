#ifndef TEXTURE_CLASS
#define TEXTURE_CLASS

#include <glad/glad.h>
#include <stb_image.h>
#include <string>
#include "error.h"

enum class TextureType {
    Diffuse,
    Specular,
    Normal,
    Emissive,
    Metallic,
    Roughness,
    Occlusion,
    Unknown
};

#include "shader.h"

class Texture {
public:
    GLuint ID;
    TextureType type;
    GLuint unit;
    bool loaded;
    std::string filePath; // Store the file path for deferred loading

    // Constructor now just stores the path - doesn't load immediately
    Texture(const char* image, TextureType texType, GLuint slot);
    
    // Move constructor and assignment for vector storage
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    
    // Delete copy constructor
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Load the texture (called when OpenGL context is ready)
    void loadTexture();
    
    void texUnit(Shader& shader, const char* uniform, GLuint unit);

    void bind();
    void unbind();

    ~Texture();

private:
    bool initialized; // Track if OpenGL object has been created
    void initializeGL(); // Create the actual OpenGL texture object
};

#endif