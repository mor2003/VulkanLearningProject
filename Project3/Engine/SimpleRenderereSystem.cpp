#include "SimpleRenderereSystem.h"

namespace Engine {
	SimpleRenderereSystem::SimpleRenderereSystem(Device& device, VkRenderPass renderPass) : device{device} {
		LoadModel();
		createDescriptorSetLayout();
		createDescriptorSets();
		createPipelineLayout();
		createGraphicsPipeline(renderPass);
	}


	SimpleRenderereSystem::~SimpleRenderereSystem() {
		vkDestroyDescriptorSetLayout(device.device(), DescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	/*
		  _  .-')     ('-.       .-') _  _ .-') _     ('-.  _  .-')     ('-.  _  .-')          .-')                  .-')    .-') _     ('-.  _   .-')    
		( \( -O )  _(  OO)     ( OO ) )( (  OO) )  _(  OO)( \( -O )  _(  OO)( \( -O )        ( OO ).               ( OO ). (  OO) )  _(  OO)( '.( OO )_  
		 ,------. (,------.,--./ ,--,'  \     .'_ (,------.,------. (,------.,------.       (_)---\_)  ,--.   ,--.(_)---\_)/     '._(,------.,--.   ,--.)
		 |   /`. ' |  .---'|   \ |  |\  ,`'--..._) |  .---'|   /`. ' |  .---'|   /`. '      /    _ |    \  `.'  / /    _ | |'--...__)|  .---'|   `.'   | 
		 |  /  | | |  |    |    \|  | ) |  |  \  ' |  |    |  /  | | |  |    |  /  | |      \  :` `.  .-')     /  \  :` `. '--.  .--'|  |    |         | 
		 |  |_.' |(|  '--. |  .     |/  |  |   ' |(|  '--. |  |_.' |(|  '--. |  |_.' |       '..`''.)(OO  \   /    '..`''.)   |  |  (|  '--. |  |'.'|  | 
		 |  .  '.' |  .--' |  |\    |   |  |   / : |  .--' |  .  '.' |  .--' |  .  '.'      .-._)   \ |   /  /\_  .-._)   \   |  |   |  .--' |  |   |  | 
		 |  |\  \  |  `---.|  | \   |   |  '--'  / |  `---.|  |\  \  |  `---.|  |\  \       \       / `-./  /.__) \       /   |  |   |  `---.|  |   |  | 
		 `--' '--' `------'`--'  `--'   `-------'  `------'`--' '--' `------'`--' '--'       `-----'    `--'       `-----'    `--'   `------'`--'   `--' 
	*/


	void SimpleRenderereSystem::LoadModel() {
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


	void SimpleRenderereSystem::createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding bindingInfo{};
		bindingInfo.binding = 0;
		bindingInfo.descriptorCount = 1;
		bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindingInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindingInfo.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo LayoutInfo{};
		LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		LayoutInfo.bindingCount = 1;
		LayoutInfo.pBindings = &bindingInfo;

		if (vkCreateDescriptorSetLayout(device.device(), &LayoutInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout");
		}
	}


	void SimpleRenderereSystem::createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAME_IN_FLIGHT, DescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAME_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();
		allocInfo.descriptorPool = device.DescriptorPool();

		DescriptorSets.resize(MAX_FRAME_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.device(), &allocInfo, DescriptorSets.data()) != VK_SUCCESS) {
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


	void SimpleRenderereSystem::createPipelineLayout() {
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &DescriptorSetLayout;

		if (vkCreatePipelineLayout(device.device(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void SimpleRenderereSystem::createGraphicsPipeline(VkRenderPass renderPass) {
		GraphicsPipelineDetails fixedFunctions = GPipeline::PipelineDefaultDetails();
		fixedFunctions.layout = pipelineLayout;
		fixedFunctions.renderPass = renderPass;
		fixedFunctions.subpass = 0;

		pipeline = std::make_unique<GPipeline>(
			device,
			"Res/Shaders/Triangle.vert.spv",
			"Res/Shaders/Triangle.frag.spv",
			fixedFunctions
		);
	}


	void SimpleRenderereSystem::RenderObject(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
		pipeline->bind(commandBuffer);
		model->Bind(commandBuffer);
		model->BindIndex(commandBuffer);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &DescriptorSets[currentFrame], 0, nullptr);
		model->Draw(commandBuffer);
	}
}