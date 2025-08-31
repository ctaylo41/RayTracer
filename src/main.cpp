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
    if (glfwGetCurrentContext() == nullptr) {
        std::cout << "No OpenGL context is current!" << std::endl;
        exit(EXIT_FAILURE);
    }
    gladLoadGL();
    if (!gladLoadGL()) {
        std::cout << "Failed to initialize OpenGL context!" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Optional: lock cursor

    

    Shader shaderProgram("/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.vert", "/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.frag");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    Scene scene("/Users/colintaylortaylor/Documents/raytracer/scenes/KhronosGroup glTF-Sample-Assets main Models-Sponza/glTF/Sponza.gltf");
    GLenum err;
    
    Camera& camera = scene.getCamera();
    glfwSetWindowUserPointer(window, &camera);

    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f;
    float fpsTimer = 0.0f;
    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Camera* cameraPtr = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPtr->ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPtr->ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPtr->ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPtr->ProcessKeyboard(RIGHT, deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFrontFace(GL_CCW);
        scene.draw(shaderProgram);
        glfwSwapBuffers(window);
        glfwPollEvents();

        // FPS calculation and window title update
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            float fps = frameCount / fpsTimer;
            std::string title = "Ray Tracer - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0;
            fpsTimer = 0.0f;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
