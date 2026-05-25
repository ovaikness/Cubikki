#pragma once

#include "Engine/Graphics/UI/Panel.hpp"
#include "Engine/Graphics/UI/TextEntryPanel.hpp"
#include "Engine/Core/DevConsole.hpp"

namespace PBE
{
	class DevConsoleGUI : public Widget
	{
	public:
		TextEntryPanel* m_pTextEntryPanel;
		TextEntryPanel* m_pBackLog;
		Panel* m_pImagePanel;

		DevConsole* m_DevConsole;
	public:
		DevConsoleGUI(DevConsole* devConsole, Window* window);
		~DevConsoleGUI();

		void Update() override;
	};
}