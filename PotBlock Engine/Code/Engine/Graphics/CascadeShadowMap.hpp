#pragma once

#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/Camera.hpp"

namespace PBE
{
	constexpr uint32_t SHADOW_MAP_RESOLUTION = 2048;
	class CascadeShadowMap
	{
	public:
		static constexpr uint32_t MAX_CASCADES = 4;
		static VkDescriptorSetLayout s_ShadowMapDescriptorSetLayout;
		static VkDescriptorPool s_ShadowMapDescriptorPool;

		Camera* m_CascadeCameras[MAX_CASCADES];
		VkSampler m_ShadowMapSampler[MAX_CASCADES];
		VkImageView m_ShadowMapImageView[MAX_CASCADES];
		VkImage m_ShadowMapImage[MAX_CASCADES];
		VmaAllocation m_ShadowMapImageAllocation[MAX_CASCADES];
		VkFramebuffer m_ShadowMapFramebuffer[MAX_CASCADES];
		VkRenderPass m_ShadowMapRenderPass[MAX_CASCADES];
		VkPipeline m_ShadowMapPipeline;
		VkPipelineLayout m_ShadowMapPipelineLayout;
	public:
		CascadeShadowMap(Renderer* renderer);
		~CascadeShadowMap();

		static void CreateShadowMapDescriptorSetLayoutAndPool(Renderer* renderer);

		void CreateShadowMapDescriptorSet(Renderer* renderer);
		void CreateShadowMapImage(Renderer* renderer);
		void CreateShadowMapFramebuffers(Renderer* renderer);
		void CreateShadowMapRenderPass(Renderer* renderer);
		void CreateShadowMapPipeline(Renderer* renderer);
		void CreateShadowMapPipelineLayout(Renderer* renderer);
		void AppendShadowMapRendersToCommandBuffer(Renderer* renderer, uint32_t cascadeIndex, VkCommandBuffer buffer, uint32_t frameIndex);
	};
}