#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <string_view>

namespace PBE
{
	class GraphicsPipelineBuilder
	{
	public:
		VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo m_RasterizationInfo;
		VkPipelineColorBlendStateCreateInfo m_ColorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo m_DepthStencilInfo;
		VkPipelineViewportStateCreateInfo m_ViewportInfo;
		VkPipelineMultisampleStateCreateInfo m_MultisampleInfo;
		std::vector<VkPipelineColorBlendAttachmentState> m_ColorBlendAttachments;
		std::vector<VkDynamicState> m_DynamicStates;
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
		std::vector<VkVertexInputAttributeDescription> m_VertexAttributeDescriptions;
		std::vector<VkVertexInputBindingDescription> m_VertexBindingDescriptions;
		VkPipelineLayout m_PipelineLayout;
		VkRenderPass m_RenderPass;
		uint32_t m_SubpassIndex = 0;

	public:
		VkPipeline CreatePipeline(VkDevice device, std::string_view debugName = "Custom Graphics Pipeline");
	};
}