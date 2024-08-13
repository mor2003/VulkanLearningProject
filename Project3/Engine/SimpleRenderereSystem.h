#pragma once

#include "Device.h"
#include "GPipeline.h"
#include "Model.h"
#include "Camera.h"

namespace Engine
{
	class SimpleRenderereSystem
	{
	public:
		SimpleRenderereSystem(Device& device, VkRenderPass renderPass, Camera& Camera);
		~SimpleRenderereSystem();

		void RenderObject(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		void UniformUpdates(uint32_t currentFrame, VkExtent2D Extent) { model->updateUniformBuffer(currentFrame, Extent); }

	private:
		void LoadModel();
		void createDescriptorSetLayout();
		void createDescriptorSets(Camera& Camera);
		void createPipelineLayout();
		void createGraphicsPipeline(VkRenderPass renderPass);

		VkDescriptorSetLayout DescriptorSetLayout;
		VkPipelineLayout pipelineLayout;

		std::vector<VkDescriptorSet> DescriptorSets;

		Device& device;
		std::unique_ptr<Model> model;
		std::unique_ptr<GPipeline> pipeline;
	};
}

