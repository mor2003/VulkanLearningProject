#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Window.h"

#include <cassert>

namespace Engine
{
	class Renderer
	{
	public:
		Renderer(Device& device, Window& window);

		VkCommandBuffer StartFrame();
		void EndFrame();

		void StartSwapchainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapchainRenderPass(VkCommandBuffer commandBuffer);

		VkRenderPass GetSwapchainRenderPass() { return swapchain->renderPass(); }
		VkExtent2D GetSwapchainExtent() { return swapchain->Extent(); }


	private:
		void recreateSwapchain();
		void AllocateCommandBuffers();
		void freeCommandBuffers();

		VkCommandBuffer GetCurrentCommandBuffer() { return commandbuffers[currentFrame]; }
		
		std::vector<VkCommandBuffer> commandbuffers;

		uint32_t ImageIndex;
		uint32_t currentFrame = 0;
		bool FrameInProgress = false;

		Window& window;
		Device& device;
		std::unique_ptr<SwapChain> swapchain;
	};
}

