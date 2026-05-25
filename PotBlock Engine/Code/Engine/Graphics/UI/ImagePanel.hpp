#pragma once

#include "Engine/Graphics/UI/Panel.hpp"
#include "Engine/Graphics/Material.hpp"

namespace PBE
{
	class ImagePanel : public Panel
	{
	public:
		ImagePanel() = default;

		std::weak_ptr<PBE::Material> m_MaterialOverride;
		AABB2 m_UVCoords;
		Rgba8 m_Tint = Rgba8::WHITE;
	public:
		void CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex);
		void Paint(CPUMesh& mesh) override;
	};
}