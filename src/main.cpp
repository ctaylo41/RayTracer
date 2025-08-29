#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "shader.h"
#include <filesystem>
#include <string>
#include "camera.h"
#include "texture.h"

const unsigned int width = 1200;
const unsigned int height = 800;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// Pyramid vertex data: position (x, y, z) and color (r, g, b)

// positions (x, y, z), colors (r, g, b), UVs (u, v)

// Each face has its own vertices and UVs for proper texture repetition
std::vector<float> pyramidVertices = {
    // Front face
    0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.5f, 1.0f, // top
   -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, // left
    0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // right

    // Right face
    0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.5f, 1.0f, // top
    0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // left (was 0,0, now 1,0)
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // right (was 1,0, now 1,1)

    // Back face
    0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.5f, 1.0f, // top
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // left (was 0,0, now 1,1)
   -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f, // right (was 1,0, now 0,1)

    // Left face
    0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.5f, 1.0f, // top
   -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f, // left (was 0,0, now 0,1)
   -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, // right

    // Base (two triangles)
   -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, // front-left
    0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // front-right
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // back-right
   -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f  // back-left
};


// Pyramid indices (each face has its own vertices)
std::vector<unsigned int> pyramidIndices = {
    // Sides
    0, 1, 2,      // front
    3, 4, 5,      // right
    6, 7, 8,      // back
    9, 10, 11,    // left
    // Base
    12, 13, 14,   // base triangle 1
    12, 14, 15    // base triangle 2
};


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    camera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

    camera->ProcessMouseScroll(yoffset);
}


int main(int, char**){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Ray Tracer", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Optional: lock cursor

    std::string vertexShaderPath = (std::filesystem::current_path().string() +  "/shaders/default.vert");
    std::string fragmentShaderPath = (std::filesystem::current_path().string() +  "/shaders/default.frag");

    std::cout << "Vertex Shader Path: " << vertexShaderPath << std::endl;
    std::cout << "Fragment Shader Path: " << fragmentShaderPath << std::endl;

    Shader shaderProgram("/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.vert", "/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.frag");

    shaderProgram.activate();

    VertexArrayObject vao;
    VertexBufferObject vbo(pyramidVertices);
    ElementBufferObject ebo(pyramidIndices);

    vao.bind();
    vbo.bind();
    ebo.bind();

    vao.linkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    vao.linkAttrib(vbo, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    vao.linkAttrib(vbo, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    Texture myTexture("/Users/colintaylortaylor/Documents/raytracer/textures/brick.png", "diffuse", 0);
    myTexture.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    myTexture.unbind();


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
    glfwSetWindowUserPointer(window, &camera);
    shaderProgram.setMat4("model", glm::value_ptr(camera.getModelMatrix()));
    shaderProgram.setMat4("view", glm::value_ptr(camera.getViewMatrix()));
    shaderProgram.setMat4("projection", glm::value_ptr(camera.getProjectionMatrix(static_cast<float>(width) / static_cast<float>(height))));
    
    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f;


    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        vao.bind();
        shaderProgram.activate();

        shaderProgram.setMat4("view", glm::value_ptr(camera.getViewMatrix()));
        shaderProgram.setMat4("projection", glm::value_ptr(camera.getProjectionMatrix((float)width/(float)height)));
        shaderProgram.setMat4("model", glm::value_ptr(camera.getModelMatrix()));

        myTexture.bind();
        myTexture.texUnit(shaderProgram, "texture1", 0); // "texture1" is the uniform name in your shader
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


        glDrawElements(GL_TRIANGLES, pyramidIndices.size(), GL_UNSIGNED_INT, 0);
        myTexture.unbind();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
