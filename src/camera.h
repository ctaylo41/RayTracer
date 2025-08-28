
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
	Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
	void updateCameraVectors();
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix(float aspectRatio);
	glm::mat4 getModelMatrix() { return glm::mat4(1.0f); }

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yoffset);

private:
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
};

#endif