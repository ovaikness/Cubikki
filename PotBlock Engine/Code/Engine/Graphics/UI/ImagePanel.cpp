#include "ImagePanel.hpp"

void PBE::ImagePanel::CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	if (!m_Visible)
	{
		return;
	}
	if (m_GPUMeshes[frameIndex])
	{
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1, &m_DescriptorSetsInFlight[frameIndex], 0, nullptr);
		if (std::shared_ptr<PBE::Material> material = m_MaterialOverride.lock())
		{
			material->CmdBind(buffer, layout, frameIndex);
		}
		else
		{
			m_pTheme->m_Material->CmdBind(buffer, layout, frameIndex);
		}
		m_GPUMeshes[frameIndex]->CmdBind(buffer);
		m_GPUMeshes[frameIndex]->CmdDraw(buffer);
	}
	Widget::CmdPostProcessDraw(buffer, layout, frameIndex);
}

void PBE::ImagePanel::Paint(CPUMesh& mesh)
{
	OBB2 bounds = GetBounds();
	Vec2 mins = bounds.GetCenter() - bounds.GetDimensions() * 0.5f;
	Vec2 maxs = bounds.GetCenter() + bounds.GetDimensions() * 0.5f;
	AABB2 boundsAABB = AABB2(mins, maxs);
	mesh.AddVertsForAABB2(boundsAABB, m_UVCoords, m_Tint);
}
