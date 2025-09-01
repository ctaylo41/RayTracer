#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <vector>
#include <string>
#include "shader.h"
#include "camera.h"

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    void draw(Shader& shader, Camera& camera);
    
    void bind();
    void unbind();

private:
    GLuint VAO, VBO;
    GLuint textureID;
    bool initialized;

    void initializeGL();
    void loadCubemapTextures(const std::vector<std::string>& faces);
    void setupCube();

};

#endif // SKYBOX_H