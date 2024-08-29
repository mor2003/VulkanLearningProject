#include "Camera.h"

namespace Engine {
	Camera::Camera(Device& device, int width, int height, glm::vec3 Position) : device{ device }, width{ width }, height{ height }, Position{ Position }
	{
		createUniformBuffers();
	}

	Camera::~Camera()
	{
		for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.device(), UniformBuffers[i], nullptr);
			vkFreeMemory(device.device(), UniformBufferMemories[i], nullptr);
		}
	}

	void Camera::Matrix(uint32_t currentFrame)
	{
		CameraUBO ubo{};
		
		ubo.view = glm::lookAt(Position, Position + Orientation, Up);
		ubo.proj = glm::perspective(glm::radians(FOV), (float)(width/height), nearPlane, farPlane);;

		ubo.proj[1][1] *= -1;
	
		memcpy(UniformData[currentFrame], &ubo, sizeof(ubo));
	}

	/*
		Position = (0.0f, 0.0f, 1.8f)
		Orientation = (0.0f, 0.0f, -1.0f)
	*/

	void Camera::Inputs(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			Position += Speed * Orientation;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			Position += Speed * -glm::normalize(glm::cross(Orientation, Up));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			Position += Speed * -Orientation;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			Position += Speed * glm::normalize(glm::cross(Orientation, Up));
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			Position += Speed * Up;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			Position += Speed * -Up;
		}


		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			cursorOn = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstClick = true;
			cursorOn = false;
		}

		if (cursorOn) {
			if (firstClick) {
				glfwSetCursorPos(window, (width / 2), (height / 2));
				firstClick = false;
			}

			double MouseX;
			double MouseY;
			glfwGetCursorPos(window, &MouseX, &MouseY);

			float RotX = sensitivity * static_cast<float>(MouseY - (height / 2)) / height;
			float RotY = sensitivity * static_cast<float>(MouseX - (height / 2)) / height;

			glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-RotX), glm::normalize(glm::cross(Orientation, Up)));

			if (!(glm::angle(newOrientation, Up) <= glm::radians(5.0f) or glm::angle(newOrientation, -Up) <= glm::radians(5.0f))) {
				Orientation = newOrientation;
			}

			Orientation = glm::rotate(Orientation, glm::radians(-RotY), Up);

			glfwSetCursorPos(window, (width / 2), (height / 2));
		}

	}

	void Camera::createUniformBuffers()
	{
		VkDeviceSize BufferSize = sizeof(CameraUBO);

		UniformBuffers.resize(MAX_FRAME_IN_FLIGHT);
		UniformBufferMemories.resize(MAX_FRAME_IN_FLIGHT);
		UniformData.resize(MAX_FRAME_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
			device.createBuffer(UniformBuffers[i], BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, UniformBufferMemories[i]);

			vkMapMemory(device.device(), UniformBufferMemories[i], 0, BufferSize, 0, &UniformData[i]);
		}
	}
}
