#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Graphics/Rgba8.hpp"
#include "Engine/Graphics/BitmapFontSystem.hpp"

namespace PBE
{
	class TextPanel : public Widget
	{
	public:
		TextPanel(std::string const& text, Vec2 const& alignment, Rgba8 const& color);
		std::string m_Text;
		float m_FontScale = 1.f;

		Vec2 m_Alignment = Vec2(0.5f, 0.5f);

		CPUMesh m_CPUMesh;
		bool m_DirtyMesh;
		std::array<bool, MAX_FRAMES_IN_FLIGHT> m_DirtyMeshSwaps{ true };
		GPUMesh* m_PresentingGPUMesh{ nullptr };
		std::vector<GPUMesh*> m_OldGPUMeshes;
		std::array<GPUMesh*, MAX_FRAMES_IN_FLIGHT> m_GPUMeshes{ nullptr };
		GPUMesh* m_GPUMesh{ nullptr };

	public:
		void CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
		void Update() override;
		void CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex) override;
		void Cleanup(Renderer* renderer) override;

		void Paint(CPUMesh& mesh) override;
	};
}