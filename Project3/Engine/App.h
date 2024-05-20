#pragma once

#include "Window.h"
#include "Device.h"
#include "Model.h"
#include "SwapChain.h"
#include "GPipeline.h"

#include <memory>

namespace Engine
{
	class App
	{
	public:
		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		static constexpr int width = 800;
		static constexpr int height = 600;

		void Run();
	private:
		// Renderer
		void recreateSwapchain();
		void AllocateCommandBuffer();
		void freeCommandBuffers();

		void DrawFrame();
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t ImageIndex);

		std::vector<VkCommandBuffer> commandBuffers;

		// Renderering Subsystems
		void LoadModel();
		void createGraphicsPipeline();
		void createDesciptorLayout();
		void createDescriptorSets();

		void createPipelineLayout();

		VkPipelineLayout PipelineLayout;
		VkDescriptorSetLayout DescriptorSetLayout;
		std::vector<VkDescriptorSet> DescriptorSets;

		Window window{ width, height };
		Device device{window};
		std::unique_ptr<SwapChain> swapchain;
		std::unique_ptr<GPipeline> pipeline;
		std::unique_ptr<Model> model;

		size_t currentFrame = 0;
	};
}