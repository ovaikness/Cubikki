#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Graphics/Camera.hpp"

#include <vector>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace PBE
{
	struct DirectionalLightCreateInfo
	{
		Vec3 m_Direction = Vec3(0.f, -1.f, 0.f);
		Vec3 m_Color = Vec3(1.f, 1.f, 1.f);
		float m_Intensity = 1.f;
		float m_AmbientIntensity = 0.1f;

		uint32_t m_ShadowMapSize = 1024;
		uint32_t m_CascadeCount = 4;

		VkDevice m_Device;
		VmaAllocator m_Allocator;
	};

	struct DirectionalLightConstants
	{
		alignas(16) Vec3 m_Direction;
		char padding[4];
		alignas(16) Vec3 m_Color;
		char padding2[4];
		alignas(4) float m_Intensity;
		alignas(4) float m_AmbientIntensity;
	};

	class DirectionalLight
	{
	public:
		static VkDescriptorSetLayout s_DescriptorSetLayout;
		static VkDescriptorPool s_DescriptorPool;

		static void CreateDescriptorSetLayout(VkDevice device);
		static void CreateDescriptorPool(VkDevice device, uint32_t count);

		DirectionalLight();

		void UpdateCSM(Camera* camera, uint32_t frameIndex);

		std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> m_DescriptorSet;

		DirectionalLightCreateInfo m_CreateInfo;
		DirectionalLightConstants m_Constants;

		std::vector<Camera> m_ShadowCascadeCameras;
		VkExtent2D m_CSMExtent;
		VkImage m_CSM;
		VkImageView m_CSMView;
		VmaAllocation m_CSMAllocation;
		std::vector<VkFramebuffer> m_CSMFramebuffers;
		std::vector<VkRenderPass> m_CSMRenderPasses;

		VkSampler m_CSMSampler;
	};
}