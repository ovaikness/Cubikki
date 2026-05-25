#include "PipelineBuilder.hpp"
#include "Engine/Graphics/GraphicsUtils.hpp"

namespace PBE
{
	VkPipeline GraphicsPipelineBuilder::CreatePipeline(VkDevice device, std::string_view debugName)
	{
		m_ColorBlendInfo.attachmentCount = (uint32_t)m_ColorBlendAttachments.size();
		m_ColorBlendInfo.pAttachments = m_ColorBlendAttachments.data();

		VkPipelineVertexInputStateCreateInfo vertexInfo = {};
		vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInfo.vertexBindingDescriptionCount = (uint32_t)m_VertexBindingDescriptions.size();
		vertexInfo.pVertexBindingDescriptions = m_VertexBindingDescriptions.data();
		vertexInfo.vertexAttributeDescriptionCount = (uint32_t)m_VertexAttributeDescriptions.size();
		vertexInfo.pVertexAttributeDescriptions = m_VertexAttributeDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint32_t)m_ShaderStages.size();
		pipelineInfo.pStages = m_ShaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInfo;
		pipelineInfo.pInputAssemblyState = &m_InputAssemblyInfo;
		pipelineInfo.pViewportState = &m_ViewportInfo;
		pipelineInfo.pRasterizationState = &m_RasterizationInfo;
		pipelineInfo.pMultisampleState = &m_MultisampleInfo;
		pipelineInfo.pDepthStencilState = &m_DepthStencilInfo;
		pipelineInfo.pColorBlendState = &m_ColorBlendInfo;

		VkPipelineDynamicStateCreateInfo m_DynamicStateInfo = {};
		m_DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		m_DynamicStateInfo.dynamicStateCount = (uint32_t)m_DynamicStates.size();
		m_DynamicStateInfo.pDynamicStates = m_DynamicStates.data();

		pipelineInfo.pDynamicState = &m_DynamicStateInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = m_SubpassIndex;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkPipeline pipeline;
		VkErrorCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Failed to create graphics pipeline!");

		VkLabelObject(device, VK_OBJECT_TYPE_PIPELINE, pipeline, debugName);

		return pipeline;
	}
}