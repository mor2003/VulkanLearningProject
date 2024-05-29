#pragma once

#include <vulkan/vulkan.h>
#include "Model.h"
#include "Device.h"

//std
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace Engine
{
	struct GraphicsPipelineDetails {
		VkPipelineInputAssemblyStateCreateInfo InputAssembly;
		VkPipelineRasterizationStateCreateInfo Rasterization;
		VkPipelineMultisampleStateCreateInfo MultiSample;
		VkPipelineColorBlendAttachmentState Attachment;
		VkPipelineColorBlendStateCreateInfo ColorBlending;
		VkPipelineDepthStencilStateCreateFlags DepthStencil;
		VkPipelineLayout layout;
		VkRenderPass renderPass;
		uint32_t subpass;
	};

	std::vector<char> ReadFile(std::string FilePath);

	const std::vector<VkDynamicState> DynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	class GPipeline
	{
		public:
			GPipeline(Device& dev, std::string VertexPath, std::string FragmentPath, const GraphicsPipelineDetails& fixedFunctions);
			~GPipeline();

			GPipeline(const GPipeline&) = delete;
			GPipeline& operator=(const GPipeline&) = delete;

			static GraphicsPipelineDetails PipelineDefaultDetails();

			void bind(VkCommandBuffer commandBuffer) { vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline); }

		private:
			void createPipeline(std::string VertexPath, std::string FragmentPath, const GraphicsPipelineDetails& fixedFunctions);

			void createShaderModule(const std::vector<char>& Code, VkShaderModule* pShaderModule);

			VkPipeline GraphicsPipeline = VK_NULL_HANDLE;

			Device& device;
	};
}

