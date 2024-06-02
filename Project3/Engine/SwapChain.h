#pragma once

#include "Window.h"
#include "Device.h"

#include <vector>
#include <stdexcept>
#include <iostream>
#include <memory>

#include <cstdlib>
#include <limits>
#include <algorithm>

namespace Engine
{

	class SwapChain
	{
		public:
			SwapChain(Device& dev, VkExtent2D windowExtent);
			SwapChain(Device& dev, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
			~SwapChain();

			SwapChain(const SwapChain&) = delete;
			SwapChain& operator=(const SwapChain&) = delete;

			VkRenderPass renderPass() { return _renderPass; }
			VkFramebuffer Framebuffer(uint32_t ImageIndex) { return framebuffers[ImageIndex]; }
			VkExtent2D Extent() { return swapchainExtent; }

			VkResult AquireNextImage(uint32_t *ImageIndex);
			VkResult SubmitCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t* ImageIndex);
			uint32_t GetImageCount() { return ImageCount; }

			VkImageView createImageView(VkImage Image, VkFormat format);

		private:
			void createSwapChain();
			void createSwapchainImageView();
			void createRenderPass();
			void createFrameBuffer();
			void createSyncObject();

			VkPresentModeKHR GetPresentMode(const std::vector<VkPresentModeKHR>& PresentModes);
			VkSurfaceFormatKHR GetSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats);
			VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

			VkSwapchainKHR swapchain;
			std::shared_ptr<SwapChain> oldSwapChain;

			std::vector<VkImage> swapchainImages;
			std::vector<VkImageView> swapchainImageViews;
			std::vector<VkFramebuffer> framebuffers;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;

			size_t currentFrame = 0;

			uint32_t ImageCount;
			VkFormat swapchainColorFormat;
			VkExtent2D swapchainExtent;

			VkRenderPass _renderPass;

			VkExtent2D windowExtent;
			Device& device;
	};
}

