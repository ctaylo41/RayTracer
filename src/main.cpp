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

const unsigned int width = 1200;
const unsigned int height = 800;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// Pyramid vertex data: position (x, y, z) and color (r, g, b)
std::vector<float> pyramidVertices = {
    // positions         // colors
     0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // top (red)
    -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f, // front-left (green)
     0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // front-right (blue)
     0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, // back-right (yellow)
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f  // back-left (magenta)
};

// Pyramid indices (4 sides + base)
std::vector<unsigned int> pyramidIndices = {
    // Sides
    0, 1, 2, // front
    0, 2, 3, // right
    0, 3, 4, // back
    0, 4, 1, // left
    // Base
    1, 2, 3,
    1, 3, 4
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

    vao.linkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    vao.linkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

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

        glDrawElements(GL_TRIANGLES, pyramidIndices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
