#pragma once

#include "Device.h"
#include "Window.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace Engine
{
	class Camera
	{
		public:
			struct CameraUBO {
				glm::mat4 view;
				glm::mat4 proj;
			};

			Camera(const Camera&) = delete;
			Camera& operator=(const Camera&) = delete;

			Camera(Device& device, int width, int height, glm::vec3 Position);
			~Camera();

			VkBuffer GetCameraBuffer(uint32_t currentFrame) { return UniformBuffers[currentFrame]; }
			
			void Matrix(uint32_t currentFrame);
			void Inputs(GLFWwindow* window);

		private:

			bool cursorOn = false;
			bool firstClick = true;

			glm::vec3 Position;
			glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
			glm::vec3 Up = glm::vec3(0.0f, -1.0f, 0.0f);

			void createUniformBuffers();

			std::vector<VkBuffer> UniformBuffers;
			std::vector<VkDeviceMemory> UniformBufferMemories;
			std::vector<void*> UniformData;

			int width, height;

			float Speed = 0.001f;
			float sensitivity = 100.f;
			
			float FOV = 45.0f;
			float nearPlane = 0.1;
			float farPlane = 100.f;

			Device& device;
	};
}

