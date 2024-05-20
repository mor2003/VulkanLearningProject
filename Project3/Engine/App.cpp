#include "App.h"

namespace Engine
{
	App::App() {
		LoadModel();
		createDesciptorLayout();
		createDescriptorSets();
		createPipelineLayout();
		recreateSwapchain();
		AllocateCommandBuffer();
	}

	App::~App() {
		vkDestroyDescriptorSetLayout(device.device(), DescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device.device(), PipelineLayout, nullptr);
	}

	void App::Run() {
		while (!window.ShouldClose()) {
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(device.device());
	}

	void App::createGraphicsPipeline() {
		GraphicsPipelineDetails fixedFunctions = GPipeline::PipelineDefaultDetails(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		fixedFunctions.layout = PipelineLayout;
		fixedFunctions.renderPass = swapchain->renderPass();
		fixedFunctions.subpass = 0;

		pipeline = std::make_unique<GPipeline>(
			device,
			"Res/Shaders/Triangle.vert.spv",
			"Res/Shaders/Triangle.frag.spv",
			fixedFunctions
		);
	}

	void App::recreateSwapchain() {
		vkDeviceWaitIdle(device.device());

		auto extent = window.WindowExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = window.WindowExtent();
			glfwWaitEvents();
		}
		

		if (swapchain == nullptr)
		{
			swapchain = std::make_unique<SwapChain>(
				device,
				window.WindowExtent()
			);
		}
		else {
			swapchain = std::make_unique<SwapChain>(
				device,
				window.WindowExtent(),
				std::move(swapchain)
			);
			if (swapchain->GetImageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				AllocateCommandBuffer();
			}
		}

		createGraphicsPipeline();
	}

	void App::createPipelineLayout() {
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &DescriptorSetLayout;

		if (vkCreatePipelineLayout(device.device(), &layoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void App::AllocateCommandBuffer() {
		commandBuffers.resize(MAX_FRAME_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = device.CommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
		{
			if (vkAllocateCommandBuffers(device.device(), &allocInfo, &commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffer");
			}
		}
	}

	void App::DrawFrame() {
		uint32_t ImageIndex = 0;

		auto result = swapchain->AquireNextImage(&ImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire image from the swap chain");
		}

		vkResetCommandBuffer(commandBuffers[currentFrame], 0);
		recordCommandBuffer(commandBuffers[currentFrame], ImageIndex);
		
		model->updateUniformBuffer(static_cast<size_t>(currentFrame), swapchain->Extent());
		result = swapchain->SubmitCommandBuffer(commandBuffers[currentFrame], &ImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.ResizedFlag()) {
			window.ResetResizedFlag();
			recreateSwapchain();
			return;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present image from the swapchain");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
	}

	void App::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t ImageIndex) {
		VkCommandBufferBeginInfo CBbeginInfo{};
		CBbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CBbeginInfo.flags = 0;
		CBbeginInfo.pNext = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &CBbeginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to record commands to command buffer");
		}

		VkRenderPassBeginInfo RPbeginInfo{};
		RPbeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RPbeginInfo.framebuffer = swapchain->Framebuffer(ImageIndex);
		RPbeginInfo.renderPass = swapchain->renderPass();

		RPbeginInfo.renderArea.offset = { 0, 0 };
		RPbeginInfo.renderArea.extent = swapchain->Extent();

		VkClearValue clearValue = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		RPbeginInfo.clearValueCount = 1;
		RPbeginInfo.pClearValues = &clearValue;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->Extent().width);
		viewport.height = static_cast<float>(swapchain->Extent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = { swapchain->Extent() };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBeginRenderPass(commandBuffer, &RPbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		pipeline->bind(commandBuffer);
		model->Bind(commandBuffer);
		model->BindIndex(commandBuffer);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSets[currentFrame], 0, nullptr);
		model->Draw(commandBuffer);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record commands to command buffer");
		}
	}

	void App::LoadModel() {
		std::vector<Model::Vertex> vertices = {
			{{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		model = std::make_unique<Model>(
			device,
			vertices,
			indices
		);
	}

	void App::freeCommandBuffers() {
		for (size_t i = 0; i < commandBuffers.size(); i++) {
			vkFreeCommandBuffers(device.device(), device.CommandPool(), 1, &commandBuffers[i]);
		}
	}

	void App::createDesciptorLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo LayoutInfo{};
		LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		LayoutInfo.bindingCount = 1;
		LayoutInfo.pBindings = &uboLayoutBinding;
		
		if (vkCreateDescriptorSetLayout(device.device(), &LayoutInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout");
		}
	}

	void App::createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAME_IN_FLIGHT, DescriptorSetLayout);
		VkDescriptorSetAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAME_IN_FLIGHT);
		AllocInfo.descriptorPool = device.DescriptorPool();
		AllocInfo.pSetLayouts = layouts.data();

		DescriptorSets.resize(MAX_FRAME_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.device(), &AllocInfo, DescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets");
		}

		for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = model->GetUniformBuffer(i);
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Model::UniformBufferObject);

			VkWriteDescriptorSet WriteSet{};
			WriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			WriteSet.dstSet = DescriptorSets[i];
			WriteSet.dstBinding = 0;
			WriteSet.dstArrayElement = 0;

			WriteSet.descriptorCount = 1;
			WriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

			WriteSet.pBufferInfo = &bufferInfo;
			WriteSet.pImageInfo = nullptr;
			WriteSet.pTexelBufferView = nullptr;
			
			vkUpdateDescriptorSets(device.device(), 1, &WriteSet, 0, nullptr);

		}
	}
}