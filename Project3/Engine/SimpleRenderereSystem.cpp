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
			/* TOP */
			{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},	// 0
			{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},		// 1
			{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},	// 2
			{{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},	// 3

			/* Bottom */
			{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},	// 4
			{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},	// 5
			{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},	// 6
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},// 7
		};

		std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0,
			0, 4, 5, 5, 1, 0,
			1, 5, 6, 6, 2, 1,
			2, 6, 7, 7, 3, 2,
			3, 7, 4, 4, 0, 3,

			4, 7, 6, 6, 5, 4
		};

		model = std::make_unique<Model>(
			device,
			vertices,
			indices
		);
	}


	void SimpleRenderereSystem::createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboBindingInfo{};
		uboBindingInfo.binding = 0;
		uboBindingInfo.descriptorCount = 1;
		uboBindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboBindingInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboBindingInfo.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding imageBindingInfo{};
		imageBindingInfo.binding = 1;
		imageBindingInfo.descriptorCount = 1;
		imageBindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageBindingInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		imageBindingInfo.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 2> bindingInfo{uboBindingInfo, imageBindingInfo};
		VkDescriptorSetLayoutCreateInfo LayoutInfo{};
		LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		LayoutInfo.bindingCount = static_cast<uint32_t>(bindingInfo.size());
		LayoutInfo.pBindings = bindingInfo.data();

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

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = model->GetTextureSampler();
			imageInfo.imageView = model->GetTextureImageView();

			std::array<VkWriteDescriptorSet, 2> WriteSet{};
			WriteSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			WriteSet[0].dstSet = DescriptorSets[i];
			WriteSet[0].dstBinding = 0;
			WriteSet[0].dstArrayElement = 0;
			WriteSet[0].descriptorCount = 1;
			WriteSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			WriteSet[0].pBufferInfo = &bufferInfo;
			WriteSet[0].pImageInfo = nullptr;
			WriteSet[0].pTexelBufferView = nullptr;

			WriteSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			WriteSet[1].dstSet = DescriptorSets[i];
			WriteSet[1].dstBinding = 1;
			WriteSet[1].dstArrayElement = 0;
			WriteSet[1].descriptorCount = 1;
			WriteSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			WriteSet[1].pBufferInfo = nullptr;
			WriteSet[1].pImageInfo = &imageInfo;
			WriteSet[1].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(device.device(), static_cast<uint32_t>(WriteSet.size()), WriteSet.data(), 0, nullptr);
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