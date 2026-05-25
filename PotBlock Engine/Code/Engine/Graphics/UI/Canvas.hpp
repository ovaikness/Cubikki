#pragma once

#include "Engine/Graphics/Camera.hpp"
#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Graphics/Window.hpp"
#include "Engine/Core/EventDispatcher.hpp"
#include "Engine/Graphics/Painters/NineSlice.hpp"
#include "Engine/Math/Vec2.hpp"

namespace PBE
{
	struct CanvasCreateInfo
	{
		Renderer* m_pRenderer;
		Window* m_pWindow;
		Theme* m_pTheme;
	};
	class Canvas : public Widget
	{
	public:
		CanvasCreateInfo m_CreateInfo;
		Camera* m_Camera;
		EventDispatcher<> m_OnUpdate;
		Widget* m_HoveredWidget = nullptr;
		Widget* m_FocusedWidget = nullptr;
	public:
		Canvas(CanvasCreateInfo const& createInfo);
		~Canvas();

		void SetFocusedWidget(Widget* widget);
		void UpdateCursorPos(Vec2 pos);

		void AddChild(Widget* child) override;

		void OnActivated() override;
		void CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
		void OnResize(float width, float height);
		void CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex);
	};
}