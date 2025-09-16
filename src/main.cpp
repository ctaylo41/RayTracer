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
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "imGuiLightManager.h"
#include "shadowManager.h"
const unsigned int width = 1200;
const unsigned int height = 800;

float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// Pyramid vertex data: position (x, y, z) and color (r, g, b)

// positions (x, y, z), colors (r, g, b), UVs (u, v)

// Each face has its own vertices and UVs for proper texture repetition


void setupSponzaLightingWithShadows(Scene& scene) {
    // Clear existing lights
    scene.getLightManager().removeAllLights();
    
    std::cout << "=== Setting up Sponza lighting with camera-based shadows ===" << std::endl;
    
    // Add a simple directional light
    size_t mainLightIndex = scene.addDirectionalLight(
        glm::normalize(glm::vec3(0.3f, -0.8f, 0.5f)), // Sun-like direction
        glm::vec3(1.0f, 0.95f, 0.8f),                 // Warm sunlight
        2.0f                                           // Intensity
    );
    
    // Enable shadows - no need to set scene bounds, it will use the camera
    scene.enableShadowsForLight(mainLightIndex, 2048);
    
    // Add a fill light (optional)
    size_t fillLightIndex = scene.addDirectionalLight(
        glm::normalize(glm::vec3(-0.2f, -0.3f, -0.8f)),
        glm::vec3(0.4f, 0.6f, 1.0f),                    // Cool fill light
        0.3f
    );
    
    std::cout << "Lighting setup complete!" << std::endl;
    std::cout << "Main light index: " << mainLightIndex << std::endl;
    std::cout << "Fill light index: " << fillLightIndex << std::endl;
    std::cout << "Shadow maps: " << scene.getShadowManager().getShadowMapCount() << std::endl;
}


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
    glfwWindowHint(GLFW_SAMPLES, 8);
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

    
    //imgui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Shader shaderProgram("/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.vert", "/Users/colintaylortaylor/Documents/raytracer/src/shaders/default.frag");
    Shader shadowShader("/Users/colintaylortaylor/Documents/raytracer/src/shaders/shadow.vert", "/Users/colintaylortaylor/Documents/raytracer/src/shaders/shadow.frag");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_MULTISAMPLE);
    Scene scene("/Users/colintaylortaylor/Documents/raytracer/scenes/KhronosGroup glTF-Sample-Assets main Models-Sponza/glTF/Sponza.gltf");
    GLenum err;
    

    scene.setSkybox("/Users/colintaylortaylor/Documents/raytracer/scenes/KhronosGroup glTF-Sample-Assets main Models-Sponza/skybox");
    scene.setSkyboxShader("/Users/colintaylortaylor/Documents/raytracer/src/shaders/skybox.vert", "/Users/colintaylortaylor/Documents/raytracer/src/shaders/skybox.frag");
    setupSponzaLightingWithShadows(scene);

    Camera& camera = scene.getCamera();
    ImGuiLightManager lightUI(scene.getLightManager(), camera);

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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        lightUI.render();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFrontFace(GL_CCW);
        scene.drawWithShadows(shaderProgram, shadowShader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

