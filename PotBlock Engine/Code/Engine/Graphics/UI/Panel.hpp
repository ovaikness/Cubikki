#pragma once

#include "Engine/Graphics/UI/Widget.hpp"

namespace PBE
{
	class Panel : public Widget
	{
	public:
		Panel();

		CPUMesh m_CPUMesh;
		bool m_DirtyMesh;
		std::array<bool, MAX_FRAMES_IN_FLIGHT> m_DirtyMeshSwaps{ true };
		GPUMesh* m_PresentingGPUMesh{ nullptr };
		std::vector<GPUMesh*> m_OldGPUMeshes;
		std::array<GPUMesh*, MAX_FRAMES_IN_FLIGHT> m_GPUMeshes{ nullptr };
		GPUMesh* m_GPUMesh{ nullptr };
	protected:
		void CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
		void Update() override;
		void CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex) override;
		void Cleanup(Renderer* renderer) override;

		void Paint(CPUMesh& mesh) override;
	};
}