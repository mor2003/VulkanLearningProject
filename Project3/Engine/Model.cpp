#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine {
	std::array<VkVertexInputAttributeDescription, 2> Model::Vertex::AttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attribDescriptions;

		attribDescriptions[0] = {};
		attribDescriptions[0].binding = 0;
		attribDescriptions[0].location = 0;
		attribDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribDescriptions[0].offset = offsetof(Vertex, position);

		attribDescriptions[1] = {};
		attribDescriptions[1].binding = 0;
		attribDescriptions[1].location = 1;
		attribDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDescriptions[1].offset = offsetof(Vertex, color);

		return attribDescriptions;
	}

	std::array<VkVertexInputBindingDescription, 1> Model::Vertex::BindingDescriptions() {
		std::array<VkVertexInputBindingDescription, 1> bindingDescriptions;

		bindingDescriptions[0] = {};
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	Model::Model(Device& dev, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) : device{ dev } {
		createTextureImage();
		createTextureImageView();
		createVertexBuffer(vertices);
		createIndexBuffer(indices);
		createUniformBuffers();
	}

	Model::~Model() {
		for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
			vkDestroyBuffer(device.device(), UniformBuffers[i], nullptr);
			vkFreeMemory(device.device(), UniformBuffersMemory[i], nullptr);
		}

		vkDestroyBuffer(device.device(), IndexBuffer, nullptr);
		vkFreeMemory(device.device(), IndexBufferMemory, nullptr);

		vkDestroyBuffer(device.device(), VertexBuffer, nullptr);
		vkFreeMemory(device.device(), VertexBufferMemory, nullptr);

		vkDestroyImageView(device.device(), TextureImageView, nullptr);

		vkDestroyImage(device.device(), TextureImage, nullptr);
		vkFreeMemory(device.device(), TextureBufferMemory, nullptr);
	}

	void Model::createVertexBuffer(const std::vector<Vertex>& vertices) {
		vertexCounts = static_cast<uint32_t>(vertices.size());
		assert(vertexCounts >= 3 && "Need to be atleast 3 vertices in the shader");
		VkDeviceSize BufferSize = sizeof(vertices[0]) * vertices.size();

		// Staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.createBuffer(
			stagingBuffer,
			BufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(device.device(), stagingBufferMemory, 0, BufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<uint32_t>(BufferSize));
		vkUnmapMemory(device.device(), stagingBufferMemory);

		// Vertex Buffer
		device.createBuffer(
			VertexBuffer,
			BufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VertexBufferMemory
		);

		copyBuffer(stagingBuffer, VertexBuffer, BufferSize);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
	}

	void Model::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		auto CommandBuffer = StartOneTimeCommand();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(CommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndOneTimeCommand(CommandBuffer);
	}

	void Model::createIndexBuffer(const std::vector<uint16_t>& indices) {
		IndexCounts = static_cast<uint32_t>(indices.size());
		VkDeviceSize BufferSize = sizeof(indices[0]) * indices.size();

		// Staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.createBuffer(
			stagingBuffer,
			BufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(device.device(), stagingBufferMemory, 0, BufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<uint32_t>(BufferSize));
		vkUnmapMemory(device.device(), stagingBufferMemory);

		// Index Buffer
		device.createBuffer(
			IndexBuffer,
			BufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			IndexBufferMemory
		);

		copyBuffer(stagingBuffer, IndexBuffer, BufferSize);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
	}

	void Model::createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		UniformBuffers.resize(MAX_FRAME_IN_FLIGHT);
		UniformBuffersMemory.resize(MAX_FRAME_IN_FLIGHT);
		UniformBuffersMapped.resize(MAX_FRAME_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
			device.createBuffer(
				UniformBuffers[i],
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				UniformBuffersMemory[i]
			);

			vkMapMemory(device.device(), UniformBuffersMemory[i], 0, bufferSize, 0, &UniformBuffersMapped[i]);
		}
	}

	void Model::updateUniformBuffer(size_t currentImage, VkExtent2D Extent) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Extent.width) / static_cast<float>(Extent.height), 0.1f, 10.f);


		ubo.proj[1][1] *= -1;

		memcpy(UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void Model::createTextureImage() {
		int Texwidth, TexHeight, TexChannel;
		stbi_uc* pixels = stbi_load("Res/Textures/statue.jpg", &Texwidth, &TexHeight, &TexChannel, STBI_rgb_alpha);

		VkDeviceSize ImageSize = Texwidth * TexHeight * 4;
		if (!pixels) {
			throw std::runtime_error("failed to load the texture pixels");
		}
		
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device.createBuffer(
			stagingBuffer,
			ImageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(device.device(), stagingBufferMemory, 0, ImageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(ImageSize));
		vkUnmapMemory(device.device(), stagingBufferMemory);

		stbi_image_free(pixels);

		device.createImage(
			TextureImage,
			{static_cast<uint32_t>(Texwidth), static_cast<uint32_t>(TexHeight)},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_R8G8B8A8_SRGB,
			ImageSize,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			TextureBufferMemory
			);

		transitionImageLayout(TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, TextureImage, static_cast<uint32_t>(Texwidth), static_cast<uint32_t>(TexHeight));
		transitionImageLayout(TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
	}

	VkCommandBuffer Model::StartOneTimeCommand() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = device.CommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		
		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(device.device(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recordin the one time command buffer");
		}

		return commandBuffer;
	}

	void Model::EndOneTimeCommand(VkCommandBuffer& CommandBuffer) {
		if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to end recordin the one time command buffer");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer;

		vkQueueSubmit(device.GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device.GraphicsQueue());

		vkFreeCommandBuffers(device.device(), device.CommandPool(), 1, &CommandBuffer);
	}

	void Model::transitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout oldImageLayout, VkImageLayout newImageLayout) {
		auto CommandBuffer = StartOneTimeCommand();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldImageLayout;
		barrier.newLayout = newImageLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}	
		else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition");
		}

		vkCmdPipelineBarrier(
			CommandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndOneTimeCommand(CommandBuffer);
	}
	
	void Model::copyBufferToImage(VkBuffer buffer, VkImage Image, uint32_t width, uint32_t height) {
		auto CommandBuffer = StartOneTimeCommand();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = 0;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			CommandBuffer,
			buffer,
			Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		EndOneTimeCommand(CommandBuffer);
	}

	void Model::createTextureImageView() {
		VkImageViewCreateInfo ViewInfo{};
		ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewInfo.image = TextureImage;
		ViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ViewInfo.subresourceRange.baseArrayLayer = 0;
		ViewInfo.subresourceRange.layerCount = 1;
		ViewInfo.subresourceRange.baseMipLevel = 0;
		ViewInfo.subresourceRange.levelCount = 1;

		if (vkCreateImageView(device.device(), &ViewInfo, nullptr, &TextureImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create an image view for texture");
		}
	}
}