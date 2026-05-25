#include "Engine/Graphics/UI/DevConsoleGUI.hpp"
#include "Engine/Graphics/UI/Canvas.hpp"
PBE::DevConsoleGUI::DevConsoleGUI(DevConsole* devConsole, Window* window)
	: m_DevConsole(devConsole)
{
	m_IsHoverable = false;
	m_Visible = false;

	m_pImagePanel = new Panel();
	m_pImagePanel->m_AnchorOffsetInPixels = AABB2(Vec2(0.f,0.f),Vec2(0.f,0.f));
	m_pImagePanel->m_NormalizedAnchors = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f));

	m_pTextEntryPanel = new TextEntryPanel(window);
	m_pTextEntryPanel->m_AnchorOffsetInPixels = AABB2(Vec2(8.f, 8.f), Vec2(-8.f, 40.f));
	m_pTextEntryPanel->m_NormalizedAnchors = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 0.f));

	m_pBackLog = new TextEntryPanel(window);
	m_pBackLog->m_AnchorOffsetInPixels = AABB2(Vec2(8.f, 44.f), Vec2(-8.f, -12.f));
	m_pBackLog->m_NormalizedAnchors = AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f));
	m_pBackLog->m_IsHoverable = false;

	m_pBackLog->m_pTextPanel->m_AnchorOffsetInPixels = AABB2(Vec2(8.f, 12.f), Vec2(-8.f, -12.f));

	m_pImagePanel->AddChild(m_pTextEntryPanel);
	m_pImagePanel->AddChild(m_pBackLog);
	AddChild(m_pImagePanel);

	window->m_OnKeyPressed.AddSubscriber([this](int key,int,int)
		{
			if (key == GLFW_KEY_ENTER)
			{
				if (!m_Visible)
				{
					m_Visible = true;
					m_Canvas->SetFocusedWidget(m_pTextEntryPanel);
					return true;
				}
				if (m_pTextEntryPanel->m_Text.empty())
				{
					m_Visible = false;
					m_Canvas->SetFocusedWidget(nullptr);
					return true;
				}
				m_DevConsole->ExecuteCommand(m_pTextEntryPanel->m_Text);
				m_pTextEntryPanel->m_Text = "";
				m_pTextEntryPanel->MarkDirty();

				m_pBackLog->m_Text = "";
				for (auto itr = m_DevConsole->m_Logs.begin(); itr != m_DevConsole->m_Logs.end(); ++itr)
				{
					m_pBackLog->m_Text += *itr + ((itr + 1 == m_DevConsole->m_Logs.end()) ? "" : "\n");
				}
				m_pBackLog->MarkDirty();

				return true;
			}

			return false;
		});

	UpdateTransform();
}

PBE::DevConsoleGUI::~DevConsoleGUI()
{
	delete m_pTextEntryPanel;
	delete m_pBackLog;
	delete m_pImagePanel;
}

void PBE::DevConsoleGUI::Update()
{
	if (!m_pTextEntryPanel->m_IsFocused)
	{
		m_Visible = false;
	}
	Widget::Update();
}

