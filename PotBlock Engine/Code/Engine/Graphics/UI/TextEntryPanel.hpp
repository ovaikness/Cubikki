#pragma once

#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Graphics/UI/TextPanel.hpp"
#include "Engine/Graphics/UI/Panel.hpp"
#include "Engine/Time/Timer.hpp"

namespace PBE
{
	class TextEntryPanel : public Widget
	{
	public:
		TextEntryPanel(Window* window);

		Timer m_CursorTimer;
		
		Window* m_pWindow = nullptr;
		std::string m_Text;
		Panel* m_pImagePanel = nullptr;
		TextPanel* m_pTextPanel = nullptr;
		Vec2 m_Alignment = Vec2(0.f, 0.f);

		bool m_CursorVisible = true;

		bool m_Highlighting = false;
		bool m_HighlightVisible = false;

		int m_HighlightStart = 0;
		int m_HighlightEnd = 0;

		int m_CursorPosition = 0;

		EventDispatcher<int> m_OnKeyTyped;

		//Cursor Mesh
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

		void OnKeyPressed(int key);
		void OnKeyReleased(int key);
		bool OnKeyTyped(int key);

		void Paint(CPUMesh& mesh) override;
	};
}