#include "GPipeline.h"

namespace Engine {
	std::vector<char> ReadFile(std::string FilePath) {
		std::ifstream File(FilePath, std::ios::ate | std::ios::binary);

		if (!File.is_open()) {
			throw std::runtime_error("failed to open file");
		}

		size_t fileSize = File.tellg();
		File.seekg(0);

		std::vector<char> buffers(fileSize);
		File.read(buffers.data(), fileSize);

		File.close();
		return buffers;
	}

	GPipeline::GPipeline(Device& dev, std::string VertexPath, std::string FragmentPath, const GraphicsPipelineDetails& fixedFunctions): device {dev}{
		createPipeline(VertexPath, FragmentPath, fixedFunctions);
	}

	GPipeline::~GPipeline() {
		vkDestroyPipeline(device.device(), GraphicsPipeline, nullptr);
	}

	void GPipeline::createPipeline(std::string VertexPath, std::string FragmentPath, const GraphicsPipelineDetails& fixedFunctions) {
		auto VertexCode = ReadFile(VertexPath);
		auto FragmentCode = ReadFile(FragmentPath);

		VkShaderModule VertexModule, FragmentModule;
		createShaderModule(VertexCode, &VertexModule);
		createShaderModule(FragmentCode, &FragmentModule);

		VkPipelineShaderStageCreateInfo ShaderStage[2];
		ShaderStage[0] = {};
		ShaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		ShaderStage[0].module = VertexModule;
		ShaderStage[0].pName = "main";

		ShaderStage[1] = {};
		ShaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		ShaderStage[1].module = FragmentModule;
		ShaderStage[1].pName = "main";

		auto AttributeDescriptions = Model::Vertex::AttributeDescriptions();
		auto BindingDescriptions = Model::Vertex::BindingDescriptions();

		VkPipelineVertexInputStateCreateInfo VertexInput{};
		VertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(AttributeDescriptions.size());
		VertexInput.pVertexAttributeDescriptions = AttributeDescriptions.data();
		VertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(BindingDescriptions.size());
		VertexInput.pVertexBindingDescriptions = BindingDescriptions.data();

		VkPipelineDynamicStateCreateInfo dynamicStates{};
		dynamicStates.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStates.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
		dynamicStates.pDynamicStates = DynamicStates.data();

		VkPipelineViewportStateCreateInfo viewState{};
		viewState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewState.viewportCount = 1;
		//viewState.pViewports = &fixedFunctions.viewport;
		viewState.scissorCount = 1;
		//viewState.pScissors = &fixedFunctions.scissor;

		VkPipelineDepthStencilStateCreateInfo DepthStencil{};
		DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencil.depthWriteEnable = VK_TRUE;
		DepthStencil.depthTestEnable = VK_TRUE;
		DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		DepthStencil.depthBoundsTestEnable = VK_FALSE;
		DepthStencil.minDepthBounds = 0.0f;
		DepthStencil.maxDepthBounds = 1.0f;
		DepthStencil.stencilTestEnable = VK_FALSE;
		DepthStencil.front = {};
		DepthStencil.back = {};

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = ShaderStage;
		
		pipelineInfo.pVertexInputState = &VertexInput;
		pipelineInfo.pDynamicState = &dynamicStates;
		pipelineInfo.pViewportState = &viewState;

		pipelineInfo.pInputAssemblyState = &fixedFunctions.InputAssembly;
		pipelineInfo.pRasterizationState = &fixedFunctions.Rasterization;
		pipelineInfo.pMultisampleState = &fixedFunctions.MultiSample;
		pipelineInfo.pColorBlendState = &fixedFunctions.ColorBlending;
		pipelineInfo.pDepthStencilState = &DepthStencil;

		pipelineInfo.layout = fixedFunctions.layout;
		pipelineInfo.renderPass = fixedFunctions.renderPass;
		pipelineInfo.subpass = fixedFunctions.subpass;

		pipelineInfo.basePipelineIndex = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline");
		}
		//std::cout << "Pipeline created" << std::endl;

		vkDestroyShaderModule(device.device(), VertexModule, nullptr);
		vkDestroyShaderModule(device.device(), FragmentModule, nullptr);
	}

	void GPipeline::createShaderModule(const std::vector<char>& Code, VkShaderModule* pShaderModule) {
		VkShaderModuleCreateInfo ModuleInfo{};
		ModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ModuleInfo.codeSize = Code.size();
		ModuleInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());

		if (vkCreateShaderModule(device.device(), &ModuleInfo, nullptr, pShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create the shader module");
		}
	}


	//uint32_t width, uint32_t height
	GraphicsPipelineDetails GPipeline::PipelineDefaultDetails() {
		GraphicsPipelineDetails pipeline;

		pipeline.InputAssembly = {};
		pipeline.InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipeline.InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline.InputAssembly.primitiveRestartEnable = VK_FALSE;

		pipeline.Rasterization = {};
		pipeline.Rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline.Rasterization.rasterizerDiscardEnable = VK_FALSE;
		pipeline.Rasterization.depthClampEnable = VK_FALSE;
		pipeline.Rasterization.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline.Rasterization.lineWidth = 1.0f;
		pipeline.Rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
		pipeline.Rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipeline.Rasterization.depthBiasEnable = VK_FALSE;
		pipeline.Rasterization.depthBiasClamp = 0.0f;
		pipeline.Rasterization.depthBiasConstantFactor = 0.0f;
		pipeline.Rasterization.depthBiasSlopeFactor = 0.0f;

		pipeline.MultiSample = {};
		pipeline.MultiSample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipeline.MultiSample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipeline.MultiSample.alphaToCoverageEnable = VK_FALSE;
		pipeline.MultiSample.alphaToOneEnable = VK_FALSE;
		pipeline.MultiSample.sampleShadingEnable = VK_FALSE;
		pipeline.MultiSample.minSampleShading = 1.0f;
		pipeline.MultiSample.pSampleMask = nullptr;

		pipeline.Attachment = {};
		pipeline.Attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline.Attachment.blendEnable = VK_FALSE;
		pipeline.Attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		pipeline.Attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipeline.Attachment.colorBlendOp = VK_BLEND_OP_ADD;
		pipeline.Attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		pipeline.Attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipeline.Attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		pipeline.ColorBlending = {};
		pipeline.ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipeline.ColorBlending.logicOpEnable = VK_FALSE;
		pipeline.ColorBlending.logicOp = VK_LOGIC_OP_COPY;
		pipeline.ColorBlending.attachmentCount = 1;
		pipeline.ColorBlending.pAttachments = &pipeline.Attachment;
		pipeline.ColorBlending.blendConstants[0] = 0.0f;
		pipeline.ColorBlending.blendConstants[1] = 0.0f;
		pipeline.ColorBlending.blendConstants[2] = 0.0f;
		pipeline.ColorBlending.blendConstants[3] = 0.0f;

		return pipeline;
	}
}