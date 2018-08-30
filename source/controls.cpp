// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow
	*window; // The "extern" keyword here is to access the variable "window"
			 // declared in tutorialXXX.cpp. This is a hack to keep the
			 // tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() { return ViewMatrix; }
glm::mat4 getProjectionMatrix() { return ProjectionMatrix; }

// Initial position : on +Z
glm::vec3 position = glm::vec3(0, .75, 0);
// Initial horizontal angle : toward -Z
float horizontalAngle = 0.0f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 30.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.002f;

void computeMatricesFromInputs()
{

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle -= mouseSpeed * float(768 / 2 - ypos);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(cos(verticalAngle) * sin(horizontalAngle),
						sin(verticalAngle),
						cos(verticalAngle) * cos(horizontalAngle));

	// Right vector
	glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.14f / 2.0f), 0,
								cos(horizontalAngle - 3.14f / 2.0f));

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	float FoV =
		initialFoV;

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit
	// <-> 100 units
	ProjectionMatrix =
		glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

	float distance = 10; // Straight line distance between the camera and look at point

	// Calculate the camera position using the distance and angles
	float camX = 0.0001465 + distance * cos(verticalAngle) * sin(horizontalAngle);
	float camY = 1.25846 + distance * sin(verticalAngle);
	float camZ = 0.0004519 + distance * cos(verticalAngle) * cos(horizontalAngle);

	// Set the camera position and lookat point
	ViewMatrix = glm::lookAt({camX, camY, camZ},			  // Camera position
							 {0.0001465, 1.25846, 0.0004519}, // Look at point
							 up);							  // Up vector

	lastTime = currentTime;
}