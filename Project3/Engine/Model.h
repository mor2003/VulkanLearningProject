#pragma once

#include "Device.h"
#include "SwapChain.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
				glm::vec3 position;
				glm::vec3 color;
				glm::vec2 texCoord;

				static std::array<VkVertexInputAttributeDescription, 3> AttributeDescriptions();
				static std::array<VkVertexInputBindingDescription, 1> BindingDescriptions();
			};

			struct UniformBufferObject {
				//alignas(16) glm::mat4 model;
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
			VkBuffer GetUniformBuffer(size_t currentFrame) { return UniformBuffers[currentFrame]; }

			VkImageView GetTextureImageView() { return TextureImageView; }
			VkSampler GetTextureSampler() { return TextureSampler; }

		private:
			void createVertexBuffer(const std::vector<Vertex>& vertices);
			void createIndexBuffer(const std::vector<uint16_t>& indices);
			void createUniformBuffers();
			void createTextureImage();
			void createTextureImageView();
			void createTextureSampler();

			void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
			void transitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
			void copyBufferToImage(VkBuffer buffer, VkImage Image, uint32_t width, uint32_t height);

			VkCommandBuffer StartOneTimeCommand();
			void EndOneTimeCommand(VkCommandBuffer& CommandBuffer);

			VkBuffer VertexBuffer;
			VkDeviceMemory VertexBufferMemory;
			VkBuffer IndexBuffer;
			VkDeviceMemory IndexBufferMemory;

			VkImage TextureImage;
			VkDeviceMemory TextureBufferMemory;
			VkImageView TextureImageView;

			VkImage DepthImage;
			VkDeviceMemory DepthbufferMemory;
			VkImageView DepthImageView;

			VkSampler TextureSampler;

			std::vector<VkBuffer> UniformBuffers;
			std::vector<VkDeviceMemory> UniformBuffersMemory;
			std::vector<void*> UniformBuffersMapped;

			Device& device;

			uint32_t vertexCounts;
			uint32_t IndexCounts;

	};
}

