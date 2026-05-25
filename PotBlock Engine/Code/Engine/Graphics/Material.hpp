#pragma once
#include "Engine/Graphics/GraphicsUtils.hpp"

#include "Engine/Graphics/Rgba8.hpp"
#include "Engine/Graphics/Texture.hpp"
#include "Engine/Graphics/GPUBuffer.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace PBE
{
	struct MaterialConstants
	{
		alignas(16) Vec4 m_DiffuseColor = Rgba8::WHITE.ToVec4();
		alignas(16) Vec4 m_SpecularColor = Rgba8::WHITE.ToVec4();
		alignas(16) Vec4 m_AmbientColor  = Rgba8::WHITE.ToVec4();

		alignas(16) float m_Shininess = 1.0f;
	};

	struct MaterialCreateInfo
	{
		VkDevice m_Device{ VK_NULL_HANDLE };
		VmaAllocator m_Allocator{ VK_NULL_HANDLE };

		Texture m_DiffuseTexture{ VK_NULL_HANDLE };
		Texture m_SpecularGlossEmitTexture{ VK_NULL_HANDLE };
		Texture m_NormalTexture{ VK_NULL_HANDLE };
		Texture m_ParallaxTexture{ VK_NULL_HANDLE };

		MaterialConstants m_MaterialConstants{};
	};

	class Material
	{
	public:
		Material(MaterialCreateInfo const& info);
		~Material();

		static void CreateDescriptorSetLayout(VkDevice device, uint32_t materialCount);
		static void DestroyDescriptorSetLayout(VkDevice device);

		//Copies the material data to the GPU if it has changed
		void UpdateConstantData();
		void CmdBind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t frameIndex, uint32_t setIndex = 0);
		
		bool m_DirtyConstants = true;
		MaterialCreateInfo m_CreateInfo;
		GPUBuffer m_MaterialDataBuffer;

		static VkDescriptorPool s_DescriptorPool;
		static VkDescriptorSetLayout s_DescriptorSetLayout;
		VkDescriptorSet m_DescriptorSets { VK_NULL_HANDLE };
	};
}