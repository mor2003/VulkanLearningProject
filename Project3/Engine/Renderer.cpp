#include "Renderer.h"

namespace Engine {
	Renderer::Renderer(Device& device, Window& window) : device{ device }, window{ window } {
		recreateSwapchain();
		AllocateCommandBuffers();
	}

	void Renderer::AllocateCommandBuffers() {
		commandbuffers.resize(MAX_FRAME_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = device.CommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
		{
			if (vkAllocateCommandBuffers(device.device(), &allocInfo, &commandbuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffer");
			}
		}
	}

	void Renderer::recreateSwapchain() {
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
			if (swapchain->GetImageCount() != commandbuffers.size()) {
				freeCommandBuffers();
				AllocateCommandBuffers();
			}
		}
	}

	void Renderer::freeCommandBuffers() {
		for (size_t i = 0; i < commandbuffers.size(); i++) {
			vkFreeCommandBuffers(device.device(), device.CommandPool(), 1, &commandbuffers[i]);
		}
	}

	VkCommandBuffer Renderer::StartFrame() {
		assert(!FrameInProgress && "can't use this function if frame already in progress");

		auto result = swapchain->AquireNextImage(&ImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapchain();
			return nullptr;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire image from the swap chain");
		}

		auto commandBuffer = GetCurrentCommandBuffer();
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo CBbeginInfo{};
		CBbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CBbeginInfo.flags = 0;
		CBbeginInfo.pNext = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &CBbeginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to record commands to command buffer");
		}

		FrameInProgress = true;

		return commandBuffer;
	}

	void Renderer::EndFrame() {
		assert(FrameInProgress && "can't use this function if frame is not in progress");
		auto commandBuffer = GetCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record commands to command buffer");
		}

		auto result = swapchain->SubmitCommandBuffer(commandBuffer, &ImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.ResizedFlag()) {
			window.ResetResizedFlag();
			recreateSwapchain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present image from the swapchain");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;

		FrameInProgress = false;
	}

	void Renderer::StartSwapchainRenderPass(VkCommandBuffer commandBuffer) {
		assert(FrameInProgress && "can't use this function if frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "commandbuffer given isn't the current commandBuffer");

		VkRenderPassBeginInfo RPbeginInfo{};
		RPbeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RPbeginInfo.framebuffer = swapchain->Framebuffer(ImageIndex);
		RPbeginInfo.renderPass = swapchain->renderPass();

		RPbeginInfo.renderArea.offset = { 0, 0 };
		RPbeginInfo.renderArea.extent = swapchain->Extent();

		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };
		RPbeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		RPbeginInfo.pClearValues = clearValues.data();

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
	}

	void Renderer::EndSwapchainRenderPass(VkCommandBuffer commandBuffer) {
		assert(FrameInProgress && "can't use this function if frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "commandbuffer given isn't the current commandBuffer");

		vkCmdEndRenderPass(commandBuffer);
	}
}