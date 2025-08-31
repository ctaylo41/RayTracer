
#ifndef CAMERA_CLASS
#define CAMERA_CLASS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera {
public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov, float far, float near, unsigned int width, unsigned int height);
	void updateCameraVectors();
	
	glm::mat4 setViewMatrix();
	glm::mat4 setProjectionMatrix(float aspectRatio);
	glm::mat4 setModelMatrix() { return glm::mat4(1.0f); }

	glm::mat4 getViewMatrix() { return viewMatrix; }
	glm::mat4 getProjectionMatrix() { return projectionMatrix; }
	glm::mat4 getModelMatrix() { return modelMatrix; }

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yoffset);




private:
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 modelMatrix;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	float yaw;
	float pitch;
	float movementSpeed;
	float mouseSensitivity;
	float zoom;
	float farPlane;
	float nearPlane;
	unsigned int width;
	unsigned int height;
};

#endif