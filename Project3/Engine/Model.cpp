#include "Model.h"

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
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = device.CommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer CommandBuffer;
		if (vkAllocateCommandBuffers(device.device(), &allocInfo, &CommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command buffer for copying buffers");
		}

		VkCommandBufferBeginInfo BeginInfo{};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(CommandBuffer, &BeginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(CommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer;

		vkQueueSubmit(device.GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device.GraphicsQueue());
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
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - StartTime).count();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Extnet.width) / static_cast<float>(Extnet.height), 0.1f, 10.f);


		ubo.proj[1][1] *= -1;

		memcpy(UniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
	}
}