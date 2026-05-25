#include "Engine/Graphics/UI/TextEntryPanel.hpp"
#include "Engine/Input/KeyCodes.hpp"
#include "Engine/Math/MathUtils.hpp"
PBE::TextEntryPanel::TextEntryPanel(Window* window)
	: m_CursorTimer(nullptr, 0.5f)
{
	m_IsHoverable = true;

	m_pImagePanel = new Panel();
	m_pImagePanel->m_NormalizedAnchors = AABB2(0, 0, 1, 1);
	m_pImagePanel->m_AnchorOffsetInPixels = AABB2(0, 0, 0, 0);
	m_pImagePanel->m_pTheme = m_pTheme;
	
	m_pTextPanel= new TextPanel("", Vec2(0, 0), Rgba8::WHITE);
	m_pTextPanel->m_CullExtentPadding = Vec2(0.f,5.f);
	m_pTextPanel->m_Alignment = m_Alignment;
	m_pTextPanel->m_NormalizedAnchors = AABB2(0, 0, 1, 1);
	m_pTextPanel->m_AnchorOffsetInPixels = AABB2(8, 8, 8, 8);
	m_pTextPanel->m_pTheme = m_pTheme;

	m_pImagePanel->AddChild(m_pTextPanel);
	AddChild(m_pImagePanel);

	m_pWindow = window;

	m_pWindow->m_OnKeyTyped.AddSubscriber([this](int key)
		{
			return OnKeyTyped(key);
		});

	m_pWindow->m_OnKeyPressed.AddSubscriber([this](int key,int, int)
		{
			OnKeyPressed(key);
			return false;
		});

	m_pWindow->m_OnKeyReleased.AddSubscriber([this](int key,int, int)
		{
			OnKeyReleased(key);
			return false;
		});

	m_pWindow->OnMouseButtonReleased.AddSubscriber([this](int button, int)
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				m_Highlighting = false;
				m_CursorPosition = m_HighlightEnd;
			}
			return false;
		});

	m_pWindow->m_OnPaste.AddSubscriber([this](std::string text)
		{
			bool consumed = false;
			for (char c : text)
			{
				consumed |= OnKeyTyped(c);
			}
			return consumed;
		});

	m_pWindow->m_OnCopy.AddSubscriber([this](std::string& text)
		{
			if (m_HighlightVisible)
			{
				size_t start = Min(m_HighlightStart, m_HighlightEnd);
				size_t end = Max(m_HighlightStart, m_HighlightEnd);

				text = std::string(m_Text.begin() + start, m_Text.begin() + end);
				return true;
			}

			return false;
		}
	);

	m_OnActivated.AddSubscriber([this]()
		{
			MarkDirty();
			m_HighlightVisible = false;
			m_Highlighting = true;
			if (m_IsFocused)
			{
				Vec2 cursorPos = m_pWindow->GetCursorPos();
				AABB2 bounds = m_pTextPanel->GetAxisAlignedBounds();
				Vec2 localCursorPos = cursorPos - bounds.m_Mins;

				float textWidth = 0.f;

				for (int i = 0; i <= m_Text.size(); i++)
				{
					std::string substr = std::string(m_Text.begin(), m_Text.begin() + Max(0,i - 1));
					std::string substr2 = std::string(m_Text.begin(), m_Text.begin() + i);
					textWidth = (m_pTheme->m_Font->GetTextWidth(substr) + m_pTheme->m_Font->GetTextWidth(substr2)) * 0.5f;
					if (textWidth > localCursorPos.x)
					{
						m_CursorPosition = Max(0,i-1);
						break;
					}
				}

				textWidth = m_pTheme->m_Font->GetTextWidth(m_Text);

				if (localCursorPos.x > textWidth)
				{
					m_CursorPosition = (int)m_Text.size();
				}
			}
			else
			{
				m_CursorPosition = (int)m_Text.size();
				SetFocused(true);
			}
			m_HighlightStart = m_CursorPosition;

			return false;
		});

	UpdateTransform();
}

void PBE::TextEntryPanel::CmdTransfer(Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_DirtyMesh)
	{
		m_OldGPUMeshes.push_back(m_PresentingGPUMesh);
		m_DirtyMeshSwaps.fill(true);
		m_PresentingGPUMesh = renderer->CmdCreateMesh(buffer, frameIndex, m_CPUMesh, "Text Entry Panel Mesh");
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

void PBE::TextEntryPanel::Update()
{
	if (m_CursorPosition > m_Text.size())
	{
		m_CursorPosition = (int)m_Text.size();
	}

	if (m_Highlighting)
	{
		Vec2 cursorPos = m_pWindow->GetCursorPos();
		AABB2 bounds = m_pTextPanel->GetAxisAlignedBounds();
		Vec2 localCursorPos = cursorPos - bounds.m_Mins;

		float textWidth = 0.f;

		for (int i = 0; i <= m_Text.size(); i++)
		{
			std::string substr = std::string(m_Text.begin(), m_Text.begin() + Max(0, i - 1));
			std::string substr2 = std::string(m_Text.begin(), m_Text.begin() + i);
			textWidth = (m_pTheme->m_Font->GetTextWidth(substr) + m_pTheme->m_Font->GetTextWidth(substr2)) * 0.5f;
			if (textWidth > localCursorPos.x)
			{
				m_HighlightEnd = Max(0, i - 1);
				break;
			}
		}

		textWidth = m_pTheme->m_Font->GetTextWidth(m_Text);

		if (localCursorPos.x > textWidth)
		{
			m_HighlightEnd = (int)m_Text.size();
		}

		if (m_HighlightEnd != m_HighlightStart && m_IsFocused)
		{
			m_HighlightVisible = true;
			m_Dirty = true;
			m_DirtyMesh = true;
		}
		else
		{
			m_HighlightVisible = false;
		}
	}

	while(m_CursorTimer.DecrementPeriodIfElapsed())
	{
		m_CursorVisible = !m_CursorVisible;
		m_Dirty = true;
		m_DirtyMesh = true;
	}

	m_pTextPanel->m_Text = m_Text;
	if (!m_Dirty)
	{
		Widget::Update();
		return;
	}

	m_Dirty = false;
	m_CPUMesh = {};
	Paint(m_CPUMesh);
	Widget::Update();
}

void PBE::TextEntryPanel::CmdPostProcessDraw(VkCommandBuffer buffer, VkPipelineLayout layout, uint32_t frameIndex)
{
	Widget::CmdPostProcessDraw(buffer, layout, frameIndex);
	if (!m_IsFocused)
	{
		return;
	}
	if (!m_Visible)
	{
		return;
	}
	if (m_GPUMeshes[frameIndex])
	{
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1, &m_DescriptorSetsInFlight[frameIndex], 0, nullptr);
		m_pTheme->m_DefaultWhiteMaterial->CmdBind(buffer, layout, frameIndex);
		m_GPUMeshes[frameIndex]->CmdBind(buffer);
		m_GPUMeshes[frameIndex]->CmdDraw(buffer);
	}
}

void PBE::TextEntryPanel::Cleanup(Renderer* renderer)
{
	m_OldGPUMeshes.push_back(m_PresentingGPUMesh);
	for (auto& mesh : m_OldGPUMeshes)
	{
		renderer->FreeGPUMesh(mesh);
		mesh = nullptr;
	}
	Widget::Cleanup(renderer);
}

void PBE::TextEntryPanel::OnKeyPressed(int)
{
}

void PBE::TextEntryPanel::OnKeyReleased(int)
{
}

bool PBE::TextEntryPanel::OnKeyTyped(int key)
{
	if (!m_IsFocused)
	{
		return false;
	}

	if (m_CursorPosition > m_Text.size())
	{
		m_CursorPosition = (int)m_Text.size();
	}

	m_CursorVisible = true;
	m_CursorTimer.Start();

	MarkDirty();
	switch (key)
	{
		case GLFW_KEY_LEFT:
			if (m_CursorPosition > 0)
			{
				m_CursorPosition--;
			}
			return true;
		case GLFW_KEY_RIGHT:
			if (m_CursorPosition < m_Text.size())
			{
				m_CursorPosition++;
			}
			return true;
		case GLFW_KEY_BACKSPACE:
			if (m_HighlightVisible)
			{
				int start = Min(m_HighlightStart, m_HighlightEnd);
				int end = Max(m_HighlightStart, m_HighlightEnd);
				m_Text.erase(m_Text.begin() + start, m_Text.begin() + end);
				m_CursorPosition = start;
				m_HighlightVisible = false;
				return true;
			}
			if (m_CursorPosition > 0)
			{
				m_Text.erase(m_Text.begin() + m_CursorPosition - 1);
				m_CursorPosition--;
			}
			return true;
	}

	if (m_HighlightVisible)
	{
		int start = Min(m_HighlightStart, m_HighlightEnd);
		int end = Max(m_HighlightStart, m_HighlightEnd);
		m_Text.erase(m_Text.begin() + start, m_Text.begin() + end);
		m_CursorPosition = start;
		m_HighlightVisible = false;
	}
	if (key == '\r' || key == '\n')
	{
		return false;
	}
	m_Text.insert(m_Text.begin() + m_CursorPosition, (char)key);
	m_CursorPosition++;
	m_OnKeyTyped(key);
	return false;
}

void PBE::TextEntryPanel::Paint(CPUMesh& mesh)
{
	if (!m_IsFocused)
	{
		return;
	}

	MarkDirty();
	m_DirtyMesh = true;

	if (m_CursorVisible && !m_HighlightVisible)
	{
		std::string substr = std::string(m_Text.begin(), m_Text.begin() + m_CursorPosition);
		float textWidth = m_pTheme->m_Font->GetTextWidth(substr);
		float textHeight = m_pTheme->m_Font->GetFontSize();

		float heightOffset = -textHeight * 0.1f;

		AABB2 axisAlignedBounds = GetAxisAlignedBounds();
		AABB2 cursorBounds = AABB2(Vec2(0, 0), Vec2(2, textHeight));
		cursorBounds.Translate(Vec2(textWidth, 0) + axisAlignedBounds.m_Mins + Vec2(8, 8 + heightOffset));

		mesh.AddVertsForAABB2(cursorBounds, AABB2(0, 0, 1, 1), Rgba8::WHITE);
	}

	if (m_HighlightVisible)
	{
		int start = Min(m_HighlightStart, m_HighlightEnd);
		int end = Max(m_HighlightStart, m_HighlightEnd);

		std::string substr = std::string(m_Text.begin(), m_Text.begin() + start);
		std::string substr2 = std::string(m_Text.begin(), m_Text.begin() + end);

		float textWidth = m_pTheme->m_Font->GetTextWidth(substr);
		float textWidth2 = m_pTheme->m_Font->GetTextWidth(substr2);

		float width = textWidth2 - textWidth;

		float textHeight = m_pTheme->m_Font->GetFontSize();

		float heightOffset = -textHeight * 0.1f;

		AABB2 axisAlignedBounds = GetAxisAlignedBounds();
		AABB2 cursorBounds = AABB2(Vec2(0, 0), Vec2(width, textHeight));

		cursorBounds.Translate(Vec2(textWidth, 0) + axisAlignedBounds.m_Mins + Vec2(8, 8 + heightOffset));

		mesh.AddVertsForAABB2(cursorBounds, AABB2(0, 0, 1, 1), Rgba8(255, 255, 255, 80));
	}

}
