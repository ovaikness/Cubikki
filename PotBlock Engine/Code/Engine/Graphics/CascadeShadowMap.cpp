#include "Engine/Graphics/CascadeShadowMap.hpp"

VkDescriptorSetLayout PBE::CascadeShadowMap::s_ShadowMapDescriptorSetLayout;

PBE::CascadeShadowMap::CascadeShadowMap(Renderer* renderer)
{

}

PBE::CascadeShadowMap::~CascadeShadowMap()
{

}

void PBE::CascadeShadowMap::CreateShadowMapDescriptorSetLayoutAndPool(Renderer* renderer)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	VkDescriptorSetLayoutBinding binding{};
	binding.binding = 0;
	binding.descriptorCount = 1;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding.pImmutableSamplers = nullptr;
	layoutInfo.pBindings = &binding;

	if (vkCreateDescriptorSetLayout(renderer->m_Device, &layoutInfo, nullptr, &s_ShadowMapDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout!");
	}

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;


	if (vkCreateDescriptorPool(renderer->m_Device, &poolInfo, nullptr, &s_ShadowMapDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void PBE::CascadeShadowMap::CreateShadowMapDescriptorSet(Renderer* renderer)
{
	VkDescriptorSetAllocateInfo info;
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = s_ShadowMapDescriptorPool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &s_ShadowMapDescriptorSetLayout;

	//vkAllocateDescriptorSets(renderer->m_Device, &info, &m_ShadowMapDescriptorSet);
}

void PBE::CascadeShadowMap::CreateShadowMapImage(Renderer* renderer)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_3D;
	imageInfo.extent.width = SHADOW_MAP_RESOLUTION;
	imageInfo.extent.height = SHADOW_MAP_RESOLUTION;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = MAX_CASCADES;
	imageInfo.format = VK_FORMAT_D32_SFLOAT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	//vmaCreateImage(renderer->m_Allocator, &imageInfo, &allocInfo, &m_ShadowMapImage, &m_ShadowMapImageAllocation, nullptr);
}

void PBE::CascadeShadowMap::CreateShadowMapPipeline(Renderer* renderer)
{

}

void PBE::CascadeShadowMap::CreateShadowMapPipelineLayout(Renderer* renderer)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &Camera::s_DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(renderer->m_Device, &pipelineLayoutInfo, nullptr, &m_ShadowMapPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

void PBE::CascadeShadowMap::CreateShadowMapRenderPass(Renderer* renderer)
{
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;

	VkAttachmentDescription attachment{};
	attachment.format = VK_FORMAT_D32_SFLOAT;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference attachmentRef{};
	attachmentRef.attachment = 0;
	attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &attachmentRef;

	renderPassInfo.pAttachments = &attachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(renderer->m_Device, &renderPassInfo, nullptr, &m_ShadowMapRenderPass[0]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass!");
	}
}

void PBE::CascadeShadowMap::CreateShadowMapFramebuffers(Renderer* renderer)
{
	for (int i = 0; i < MAX_CASCADES; ++i)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_ShadowMapRenderPass[i];
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &m_ShadowMapImageView[i];
		framebufferInfo.width = SHADOW_MAP_RESOLUTION;
		framebufferInfo.height = SHADOW_MAP_RESOLUTION;
		framebufferInfo.layers = MAX_CASCADES;

		if (vkCreateFramebuffer(renderer->m_Device, &framebufferInfo, nullptr, &m_ShadowMapFramebuffer[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

}

void PBE::CascadeShadowMap::AppendShadowMapRendersToCommandBuffer(Renderer* renderer, uint32_t cascadeIndex,VkCommandBuffer commandBuffer, uint32_t frameIndex)
{
	for (int i = 0; i < MAX_CASCADES; ++i)
	{
		m_CascadeCameras[i]->CmdTransfer(commandBuffer, frameIndex);
	}

	for (int i = 0; i < MAX_CASCADES; ++i)
	{
		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.renderPass = m_ShadowMapRenderPass[i];
		beginInfo.framebuffer = m_ShadowMapFramebuffer[i];
		beginInfo.renderArea.offset = { 0, 0 };
		beginInfo.renderArea.extent = { SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION };

		vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowMapPipeline);
		m_CascadeCameras[i]->CmdBind(commandBuffer, m_ShadowMapPipelineLayout, frameIndex,0);
		//Draw Stuff
		vkCmdEndRenderPass(commandBuffer);
	}

}

