#include "Engine/Graphics/UI/TextPanel.hpp"

PBE::TextPanel::TextPanel(std::string const& text, Vec2 const& alignment, Rgba8 const& color)
	: m_Text(text)
	, m_Alignment(alignment)
{
	m_Color = color;
}

void PBE::TextPanel::CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_DirtyMesh)
	{
		m_OldGPUMeshes.push_back(m_PresentingGPUMesh);
		m_DirtyMeshSwaps.fill(true);
		m_PresentingGPUMesh = renderer->CmdCreateMesh(buffer, frameIndex, m_CPUMesh, "Text Panel Mesh");
		m_DirtyMesh = false;
	}

	if (m_DirtyMeshSwaps[frameIndex])
	{
		m_GPUMeshes[frameIndex] = m_PresentingGPUMesh;
		m_DirtyMeshSwaps[frameIndex] = false;
	}

	for (auto itr = m_OldGPUMeshes.begin(); itr != m_OldGPUMeshes.end();)
	{
		GPUMesh*& mesh = *itr;
		if (std::find(m_GPUMeshes.begin(), m_GPUMeshes.end(), mesh) == m_GPUMeshes.end())
		{
			renderer->FreeGPUMesh(mesh);
			itr = m_OldGPUMeshes.erase(itr);
		}
		else
		{
			++itr;
		}
	}

	Widget::CmdTransfer(renderer, buffer, frameIndex);
}

void PBE::TextPanel::Update()
{
	if (!m_Dirty)
	{
		Widget::Update();
		return;
	}
	m_DirtyMesh = true;
	m_Dirty = false;
	m_CPUMesh = {};
	Paint(m_CPUMesh);
	Widget::Update();
}

void PBE::TextPanel::CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	if (!m_Visible)
	{
		return;
	}
	if (m_GPUMeshes[frameIndex])
	{
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1, &m_DescriptorSetsInFlight[frameIndex], 0, nullptr);
		m_pTheme->m_Font->GetMaterial()->CmdBind(buffer, layout, frameIndex);
		m_GPUMeshes[frameIndex]->CmdBind(buffer);
		m_GPUMeshes[frameIndex]->CmdDraw(buffer);
	}
	Widget::CmdPostProcessDraw(buffer, layout, frameIndex);

}

void PBE::TextPanel::Cleanup(Renderer* renderer)
{
	m_OldGPUMeshes.push_back(m_PresentingGPUMesh);
	for (auto& mesh : m_OldGPUMeshes)
	{
		renderer->FreeGPUMesh(mesh);
	}

	Widget::Cleanup(renderer);
}

void PBE::TextPanel::Paint(CPUMesh& mesh)
{
	if (!m_Visible)
	{
		return;
	}

	OBB2 bounds = GetBounds();
	Vec2 mins = bounds.GetCenter() - bounds.GetDimensions() * 0.5f;
	Vec2 maxs = bounds.GetCenter() + bounds.GetDimensions() * 0.5f;
	AABB2 boundsAABB = AABB2(mins, maxs);

	m_pTheme->m_Font->AddVertsForTextInBox(mesh,m_Text, boundsAABB, m_Alignment, m_Color, m_FontScale);

	Widget::Paint(mesh);
}
