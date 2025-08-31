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
#include "scene.h"

const unsigned int width = 1200;
const unsigned int height = 800;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// Pyramid vertex data: position (x, y, z) and color (r, g, b)

// positions (x, y, z), colors (r, g, b), UVs (u, v)

// Each face has its own vertices and UVs for proper texture repetition


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

    Scene scene("path/to/your/model.gltf");
    
    std::string vertexShaderPath = (std::filesystem::current_path().string() +  "/shaders/default.vert");
    std::string fragmentShaderPath = (std::filesystem::current_path().string() +  "/shaders/default.frag");

    std::cout << "Vertex Shader Path: " << vertexShaderPath << std::endl;
    std::cout << "Fragment Shader Path: " << fragmentShaderPath << std::endl;

    Shader shaderProgram("/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.vert", "/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.frag");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    
    Camera camera = scene.getCamera();
    glfwSetWindowUserPointer(window, &camera);

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
        


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
