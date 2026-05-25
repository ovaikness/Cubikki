#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Renderer.hpp"

#include <array>

VkDescriptorSetLayout PBE::Material::s_DescriptorSetLayout;
VkDescriptorPool PBE::Material::s_DescriptorPool;

PBE::Material::Material( MaterialCreateInfo const& createInfo )
	: m_CreateInfo( createInfo )
{
	MaterialConstants& materialData = m_CreateInfo.m_MaterialConstants;
	
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = s_DescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &s_DescriptorSetLayout;

	VkResult result = vkAllocateDescriptorSets(m_CreateInfo.m_Device, &allocInfo, &m_DescriptorSets);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	m_MaterialDataBuffer = CreateGPUBuffer(m_CreateInfo.m_Allocator, sizeof(MaterialConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	m_MaterialDataBuffer.m_Size = sizeof(MaterialConstants);
	vmaMapMemory(m_CreateInfo.m_Allocator, m_MaterialDataBuffer.m_Allocation, &m_MaterialDataBuffer.m_Data);
	memcpy(m_MaterialDataBuffer.m_Data, &materialData, sizeof(MaterialConstants));

	VkDescriptorBufferInfo materialBufferInfo = {};
	materialBufferInfo.buffer = m_MaterialDataBuffer.m_Buffer;
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = sizeof(MaterialConstants);

	std::vector<VkWriteDescriptorSet> descriptorWrites;

	VkWriteDescriptorSet materialDataWrite = {};
	materialDataWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	materialDataWrite.dstSet = m_DescriptorSets;
	materialDataWrite.dstBinding = 0;
	materialDataWrite.dstArrayElement = 0;
	materialDataWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	materialDataWrite.descriptorCount = 1;
	materialDataWrite.pBufferInfo = &materialBufferInfo;

	descriptorWrites.push_back(materialDataWrite);

	VkDescriptorImageInfo imageInfos[4] = {};
	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[0].imageView = m_CreateInfo.m_DiffuseTexture->m_ImageView;
	imageInfos[0].sampler = m_CreateInfo.m_DiffuseTexture->m_Sampler;

	imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[1].imageView = m_CreateInfo.m_SpecularGlossEmitTexture->m_ImageView;
	imageInfos[1].sampler = m_CreateInfo.m_SpecularGlossEmitTexture->m_Sampler;

	imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[2].imageView = m_CreateInfo.m_NormalTexture->m_ImageView;
	imageInfos[2].sampler = m_CreateInfo.m_NormalTexture->m_Sampler;

	imageInfos[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[3].imageView = m_CreateInfo.m_ParallaxTexture->m_ImageView;
	imageInfos[3].sampler = m_CreateInfo.m_ParallaxTexture->m_Sampler;

	VkWriteDescriptorSet textureWrite = {};
	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.dstSet = m_DescriptorSets;
	textureWrite.dstBinding = 1;
	textureWrite.dstArrayElement = 0;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = 1;
	textureWrite.pImageInfo = &imageInfos[0];

	descriptorWrites.push_back(textureWrite);

	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.dstSet = m_DescriptorSets;
	textureWrite.dstBinding = 2;
	textureWrite.dstArrayElement = 0;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = 1;
	textureWrite.pImageInfo = &imageInfos[1];

	descriptorWrites.push_back(textureWrite);

	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.dstSet = m_DescriptorSets;
	textureWrite.dstBinding = 3;
	textureWrite.dstArrayElement = 0;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = 1;
	textureWrite.pImageInfo = &imageInfos[2];

	descriptorWrites.push_back(textureWrite);

	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.dstSet = m_DescriptorSets;
	textureWrite.dstBinding = 4;
	textureWrite.dstArrayElement = 0;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = 1;
	textureWrite.pImageInfo = &imageInfos[3];

	descriptorWrites.push_back(textureWrite);

	vkUpdateDescriptorSets(m_CreateInfo.m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	VkLabelObject(m_CreateInfo.m_Device, VK_OBJECT_TYPE_DESCRIPTOR_SET, m_DescriptorSets, "Material Descriptor Set");
	VkLabelObject(m_CreateInfo.m_Device, VK_OBJECT_TYPE_BUFFER, m_MaterialDataBuffer.m_Buffer, "Material Data Buffer");
}

PBE::Material::~Material()
{
	vmaUnmapMemory(m_CreateInfo.m_Allocator, m_MaterialDataBuffer.m_Allocation);
	vmaDestroyBuffer(m_CreateInfo.m_Allocator, m_MaterialDataBuffer.m_Buffer, m_MaterialDataBuffer.m_Allocation);

	vkFreeDescriptorSets(m_CreateInfo.m_Device, s_DescriptorPool, 1, &m_DescriptorSets);
}

void PBE::Material::CreateDescriptorSetLayout(VkDevice device, uint32_t materialCount)
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = materialCount * 5;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount  = 1;
	poolInfo.pPoolSizes     = &poolSize;
	poolInfo.maxSets		= materialCount;
	poolInfo.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VkErrorCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &s_DescriptorPool), "Failed to create descriptor pool");
	VkLabelObject(device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, s_DescriptorPool, "Material Descriptor Pool");

	std::array<VkDescriptorSetLayoutBinding, 5> bindings = {};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	bindings[2].binding = 2;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[2].descriptorCount = 1;
	bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	bindings[3].binding = 3;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[3].descriptorCount = 1;
	bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[3].pImmutableSamplers = nullptr;

	bindings[4].binding = 4;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[4].descriptorCount = 1;
	bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[4].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)bindings.size();
	layoutInfo.pBindings = bindings.data();

	VkErrorCheck(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &s_DescriptorSetLayout), "Failed to create descriptor set layout");
	VkLabelObject(device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, s_DescriptorSetLayout, "Material Descriptor Set Layout");
}

void PBE::Material::DestroyDescriptorSetLayout(VkDevice device)
{
	vkDestroyDescriptorPool(device, s_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, s_DescriptorSetLayout, nullptr);
}

void PBE::Material::UpdateConstantData()
{
	//Copy the material data to the GPU if it has changed since the last frame.
	//Only run before rendering the object, not in the render loop.
	// 
	//Doing so would cause UB because the buffer is being written to while it is being read.
	if (!m_DirtyConstants)
	{
		return;
	}

	MaterialConstants& materialData = m_CreateInfo.m_MaterialConstants;
	memcpy(m_MaterialDataBuffer.m_Data, &materialData, sizeof(MaterialConstants));

	m_DirtyConstants = false;
}

void PBE::Material::CmdBind(VkCommandBuffer commandBuffer,VkPipelineLayout layout, uint32_t /*frameIndex*/, uint32_t setIndex)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setIndex, 1, &m_DescriptorSets, 0, nullptr);
}
