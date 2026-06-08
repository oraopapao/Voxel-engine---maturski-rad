#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 position) {
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, GLuint matrixLoc, const char* uniformName) {
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	view = glm::lookAt(Position, Position + Orientation, Up);
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);

	glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(projection * view));
}

glm::mat4 Camera::GetProjView(float FOVdeg, float nearPlane, float farPlane) {
	glm::mat4 view = glm::lookAt(Position, Position + Orientation, Up);
	glm::mat4 projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);
	return projection * view;
}

void Camera::Inputs(GLFWwindow* window, float deltaTime) {
	float velocity = speed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		Position += velocity * Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		Position += velocity * -glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		Position += velocity * -Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		Position += velocity * glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		Position += velocity * Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		Position += velocity * -Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		speed = 150;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
		speed = 10;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		// Sakrij kursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (firstClick) {
			firstClick = false;
			glfwSetCursorPos(window, (width / 2), (height / 2));
		}
		

	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		// Otkrij kursor kad pustiš levi klik
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
	if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED){

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Računanje rotacije (normalizovano u odnosu na centar ekrana)
		float rotX = sensitivity * (float)(mouseY - (height / 2.0)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2.0)) / width;

		// Izračunaj novu orijentaciju za pogled gore-dole (Pitch)
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		// Ograniči pogled da ne možeš da slomiš vrat (ugao ne sme proći 90 stepeni)
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f)) {
			Orientation = newOrientation;
		}

		// Rotacija levo-desno (Yaw)
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		// Vrati miš na centar ekrana da ne bi izašao van prozora
		glfwSetCursorPos(window, (width / 2.0), (height / 2.0));
	}

}
