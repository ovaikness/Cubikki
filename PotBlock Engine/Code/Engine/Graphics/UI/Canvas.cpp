#include "Engine/Graphics/UI/Canvas.hpp"

PBE::Canvas::Canvas(CanvasCreateInfo const& createInfo)
{
	m_CreateInfo = createInfo;
	m_IsHoverable = false;
	m_IsFocused = false;
	m_Camera = createInfo.m_pRenderer->CreateCamera();
	m_Camera->m_ViewportMode = ViewportMode::STRETCH;

	Window* window = createInfo.m_pWindow;
	window->m_OnResize.AddSubscriber([this](uint32_t width, uint32_t height)
		{
			OnResize((float)width, (float)height);
			return false;
		});

	m_Camera->CreateOrthographic(AABB2(0,0,window->GetWidth(), window->GetHeight()), 0.f, 100.f);
	m_Camera->OnResize((uint32_t)window->GetWidth(), (uint32_t)window->GetHeight());
	m_NormalizedAnchors = AABB2(0, 0, 0, 0);
	m_AnchorOffsetInPixels = AABB2(0,0,window->GetWidth(), window->GetHeight());
	m_pTheme = createInfo.m_pTheme;
}

PBE::Canvas::~Canvas()
{
	delete m_Camera;
	Cleanup(m_CreateInfo.m_pRenderer);
}

void PBE::Canvas::SetFocusedWidget(Widget* widget)
{
	if (m_FocusedWidget)
	{
		m_FocusedWidget->SetFocused(false);
		if (m_FocusedWidget == m_HoveredWidget)
		{
			m_FocusedWidget->SetHovered(false);
		}
	}
	m_FocusedWidget = widget;
	if (m_FocusedWidget)
	{
		m_FocusedWidget->SetFocused(true);
	}
}

void PBE::Canvas::UpdateCursorPos(Vec2 pos)
{
	Widget* currentHovered = m_HoveredWidget;
	m_HoveredWidget = GetHoveredWidget(pos);

	if (m_HoveredWidget != currentHovered)
	{
		if (currentHovered)
		{
			currentHovered->SetHovered(false);
		}
		if (m_HoveredWidget)
		{
			m_HoveredWidget->SetHovered(true);
		}
	}
}

void PBE::Canvas::AddChild(Widget* child)
{
	child->SetCanvas(this);
	child->SetParent(this);
	child->SetTheme(m_pTheme);
	auto it = std::find(m_Children.begin(), m_Children.end(), child);
	if (it != m_Children.end())
	{
		return;
	}

	auto nIt = std::find(m_Children.begin(), m_Children.end(), nullptr);
	if (nIt != m_Children.end())
	{
		*nIt = child;
	}
	else
	{
		m_Children.push_back(child);
	}

	UpdateTransform();
}

void PBE::Canvas::OnActivated()
{
	if (m_HoveredWidget)
	{
		SetFocusedWidget(m_HoveredWidget);
		m_HoveredWidget->OnActivated();
	}
	else
	{
		SetFocusedWidget(nullptr);
	}
}

void PBE::Canvas::CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	m_Camera->CmdTransfer(buffer, frameIndex);
	Widget::CmdTransfer(renderer, buffer, frameIndex);
}

void PBE::Canvas::OnResize(float width, float height)
{
	m_Camera->CreateOrthographic(AABB2(0, 0, width, height), 0.f, 100.f);
	m_Camera->OnResize((uint32_t)width, (uint32_t)height);
	m_NormalizedAnchors = AABB2(0, 0, 0, 0);
	m_AnchorOffsetInPixels = AABB2(0, 0, width, height);

	MarkDirty();
}

void PBE::Canvas::CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	m_Camera->CmdBind(buffer, layout, frameIndex, 1);
	PBE::Widget::CmdPostProcessDraw(buffer, layout, frameIndex);
}
