#pragma once

#include "Device.h"
#include "SwapChain.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cassert>
#include <chrono>

namespace Engine
{
	class Model
	{
		public:
			struct Vertex {
				glm::vec2 position;
				glm::vec3 color;

				static std::array<VkVertexInputAttributeDescription, 2> AttributeDescriptions();
				static std::array<VkVertexInputBindingDescription, 1> BindingDescriptions();
			};

			struct UniformBufferObject {
				alignas(16) glm::mat4 model;
				alignas(16) glm::mat4 view;
				alignas(16) glm::mat4 proj;
			};
			
			Model(Device& dev, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
			~Model();

			Model(const Model&) = delete;
			Model& operator=(const Model&) = delete;

			void Bind(VkCommandBuffer CommandBuffer) { 
				VkBuffer VertexBuffers[] = { VertexBuffer };
				VkDeviceSize offsets[] = { 0 };

				vkCmdBindVertexBuffers(CommandBuffer, 0, 1, VertexBuffers, offsets);
			}

			void BindIndex(VkCommandBuffer CommandBuffer) { vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer, 0, VK_INDEX_TYPE_UINT16); }
			void Draw(VkCommandBuffer CommandBuffers) { vkCmdDrawIndexed(CommandBuffers, IndexCounts, 1, 0, 0, 0); }

			void updateUniformBuffer(size_t currentImage, VkExtent2D Extent);
			VkBuffer GetUniformBuffer(size_t currentFrame) { return uniformBuffer[currentFrame]; }

		private:
			void createVertexBuffer(const std::vector<Vertex>& vertices);
			void createIndexBuffer(const std::vector<uint16_t>& indices);
			void createUniformBuffers();

			void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

			VkBuffer VertexBuffer;
			VkDeviceMemory VertexBufferMemory;
			VkBuffer IndexBuffer;
			VkDeviceMemory IndexBufferMemory;

			std::vector<VkBuffer> uniformBuffer;
			std::vector<VkDeviceMemory> uniformBufferMemory;
			std::vector<void*> uniformBufferMapped;
			std::vector<VkDescriptorSet> DescriptorSets;

			Device& device;

			uint32_t vertexCounts;
			uint32_t IndexCounts;

	};
}

