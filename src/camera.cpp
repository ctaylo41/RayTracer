#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera() 
{
    // Default constructor implementation (can be left empty or initialize members if needed)
}

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov, float far, float near, unsigned int width, unsigned int height)
    : position(position), worldUp(up), yaw(yaw), pitch(pitch), movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(fov), farPlane(far), nearPlane(near), width(width), height(height)
{
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    updateCameraVectors();
    viewMatrix = setViewMatrix();
    projectionMatrix = setProjectionMatrix(static_cast<float>(width) / static_cast<float>(height));
    modelMatrix = setModelMatrix();
}

glm::mat4 Camera::setViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::setProjectionMatrix(float aspectRatio) {
    return glm::perspective(glm::radians(zoom), aspectRatio, nearPlane, farPlane);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (direction == FORWARD)
        position += front * velocity;
    if (direction == BACKWARD)
        position -= front * velocity;
    if (direction == LEFT)
        position -= right * velocity;
    if (direction == RIGHT)
        position += right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}



