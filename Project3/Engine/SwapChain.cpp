#include "SwapChain.h"

namespace Engine {
	SwapChain::SwapChain(Device& dev, VkExtent2D windowExtent) : device{ dev }, windowExtent{windowExtent} {
		createSwapChain();
		createSwapchainImageView();
		createRenderPass();
		createFrameBuffer();
		createSyncObject();
	}

	SwapChain::SwapChain(Device& dev, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous) : device{ dev }, windowExtent{ windowExtent }, oldSwapChain{previous} {
		createSwapChain();
		createSwapchainImageView();
		createRenderPass();
		createFrameBuffer();
		createSyncObject();

		oldSwapChain = nullptr;
	}

	SwapChain::~SwapChain() {

		for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device.device(), inFlightFences[i], nullptr);
		}


		for (const auto& framebuffer : framebuffers) {
			vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(device.device(), _renderPass, nullptr);

		for (const auto& imageView : swapchainImageViews) {
			vkDestroyImageView(device.device(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(device.device(), swapchain, nullptr);
	}

	VkPresentModeKHR SwapChain::GetPresentMode(const std::vector<VkPresentModeKHR>& PresentModes) {
		for (const auto& PresentMode : PresentModes) {
			if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "\nPresent Mode Use: MAILBOX" << std::endl;
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}

		std::cout << "\nPresent Mode Use: FIFO" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkSurfaceFormatKHR SwapChain::GetSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats) {
		for (const auto& Format : Formats) {
			if (Format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR && Format.format == VK_FORMAT_R8G8B8A8_SRGB) {
				return Format;
			}
		}

		return Formats[0];
	}

	VkExtent2D SwapChain::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = windowExtent;

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void SwapChain::createSwapChain() {
		SwapChainSupportDetails details = device.GetSwapchainDetails();

		VkExtent2D extent = chooseSwapchainExtent(details.capabilities);
		VkPresentModeKHR PresentMode = GetPresentMode(details.presentModes);
		VkSurfaceFormatKHR surfaceFormat = GetSurfaceFormat(details.Formats);

		uint32_t imageCount = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
			imageCount = details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = device.surface();

		swapchainInfo.minImageCount = imageCount;
		swapchainInfo.imageExtent = extent;
		swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainInfo.imageFormat = surfaceFormat.format;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = device.GetFamilyIndices();
		uint32_t queueFamilyIndices[] = {indices.graphicsQueue.value(), indices.presentQueue.value()};

		if (indices.graphicsQueue != indices.presentQueue) {
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		}
		else {
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainInfo.queueFamilyIndexCount = 0;
			swapchainInfo.pQueueFamilyIndices = nullptr;
		}

		swapchainInfo.preTransform = details.capabilities.currentTransform;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		swapchainInfo.presentMode = PresentMode;
		swapchainInfo.clipped = VK_TRUE;

		swapchainInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE: oldSwapChain->swapchain;

		if (vkCreateSwapchainKHR(device.device(), &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create a swapchain");
		}

		vkGetSwapchainImagesKHR(device.device(), swapchain, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device.device(), swapchain, &imageCount, swapchainImages.data());

		ImageCount = imageCount;
		swapchainColorFormat = surfaceFormat.format;
		swapchainExtent = extent;

		//std::cout << "Images size: " << swapchainImages.size() << std::endl;
		std::cout << "Created a swapchain" << std::endl;
	}

	void SwapChain::createSwapchainImageView() {
		swapchainImageViews.resize(swapchainImages.size());

		for (size_t i = 0; i < swapchainImageViews.size(); i++) {
			swapchainImageViews[i] = createImageView(swapchainImages[i], swapchainColorFormat);
		}
	}

	void SwapChain::createRenderPass() {
		VkAttachmentDescription ColorAttachments{};
		ColorAttachments.samples = VK_SAMPLE_COUNT_1_BIT;
		ColorAttachments.format = swapchainColorFormat;
		ColorAttachments.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		ColorAttachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ColorAttachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ColorAttachments.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ColorAttachments.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference ColorAttachmentRef{};
		ColorAttachmentRef.attachment = 0;
		ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassInfo{};
		subpassInfo.colorAttachmentCount = 1;
		subpassInfo.pColorAttachments = &ColorAttachmentRef;

		VkSubpassDependency subpassDep{};
		subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDep.dstSubpass = 0;

		subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDep.srcAccessMask = 0;

		subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo PassInfo{};
		PassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		PassInfo.attachmentCount = 1;
		PassInfo.pAttachments = &ColorAttachments;
		PassInfo.subpassCount = 1;
		PassInfo.pSubpasses = &subpassInfo;
		PassInfo.dependencyCount = 1;
		PassInfo.pDependencies = &subpassDep;

		if (vkCreateRenderPass(device.device(), &PassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create the render pass");
		}

	}

	void SwapChain::createFrameBuffer() {
		framebuffers.resize(swapchainImageViews.size());

		for (size_t i = 0; i < swapchainImageViews.size(); i++) {
			VkImageView attachment[] = {
				swapchainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachment;
			framebufferInfo.renderPass = _renderPass;
			framebufferInfo.layers = 1;
			framebufferInfo.width = swapchainExtent.width;
			framebufferInfo.height = swapchainExtent.height;
			
			if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer");
			}
		}
	}

	void SwapChain::createSyncObject() {
		imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAME_IN_FLIGHT);

		VkSemaphoreCreateInfo SemaphoreInfo{};
		SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		SemaphoreInfo.flags = 0;
		SemaphoreInfo.pNext = nullptr;

		VkFenceCreateInfo FenceInfo{};
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		FenceInfo.pNext = nullptr;

		for (uint32_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
		{
			if (
				vkCreateSemaphore(device.device(), &SemaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device.device(), &SemaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device.device(), &FenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS
				) {
				throw std::runtime_error("Sync objects failed to create");
			}
		}

	}

	VkResult SwapChain::AquireNextImage(uint32_t *ImageIndex) {
		vkWaitForFences(device.device(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);

		VkResult result = vkAcquireNextImageKHR(device.device(), swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, ImageIndex);

		return result;
	}

	VkResult SwapChain::SubmitCommandBuffer(VkCommandBuffer CommandBuffer, uint32_t* ImageIndex) {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer;

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(device.GraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = ImageIndex;

		presentInfo.pResults = nullptr;

		currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;

		return vkQueuePresentKHR(device.PresentQueue(), &presentInfo);
	}

	VkImageView SwapChain::createImageView(VkImage Image, VkFormat format) {
		VkImageViewCreateInfo ViewInfo{};
		ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewInfo.image = Image;
		ViewInfo.format = format;
		ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ViewInfo.subresourceRange.levelCount = 1;
		ViewInfo.subresourceRange.baseMipLevel = 0;
		ViewInfo.subresourceRange.layerCount = 1;
		ViewInfo.subresourceRange.baseArrayLayer = 0;

		VkImageView ImageView;
		if (vkCreateImageView(device.device(), &ViewInfo, nullptr, &ImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create an image view");
		}

		return ImageView;
	}
}