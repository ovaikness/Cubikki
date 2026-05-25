#include "Engine/Core/DevConsole.hpp"
#include "Engine/IO/FileUtils.hpp"
#include "Engine/Input/KeyCodes.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <iostream>

namespace PBE
{
	DevConsole::DevConsole(DevConsoleConfig const& config)
		: m_Config(config)
	{
		//m_Config.m_pWindow->m_OnKeyPressed.AddSubscriber([this](int key, int, int) 
		//	{
		//		KeyPressed(key);
		//		return true;
		//	});
		//
		//m_Config.m_pWindow->m_OnKeyReleased.AddSubscriber([this](int key, int, int)
		//	{
		//		KeyReleased(key);
		//		return true;
		//	});
		//m_Config.m_pWindow->m_OnKeyTyped.AddSubscriber([this](int key)
		//	{
		//		KeyTyped(key);
		//		return true;
		//	});

		SetupCommands();
	}

	DevConsole::~DevConsole()
	{
	}


	void DevConsole::AddLog(std::string_view message)
	{
		std::lock_guard<std::mutex> lock(m_LogMutex);
		std::cout << std::string(message) << std::endl;
		m_Logs.push_back(std::string(message));
	}

	void DevConsole::AddWarning(std::string_view message)
	{
		std::lock_guard<std::mutex> lock(m_LogMutex);
		std::cout << "Warning: " << std::string(message) << std::endl;
		m_Logs.push_back("Warning: " + std::string(message));
		__debugbreak();
	}

	void DevConsole::ExecuteCommand(std::string_view command)
	{
		AddLog(command);
		auto parts = SplitString(command);

		if (parts.empty())
		{
			return;
		}

		auto it = m_Commands.find(parts[0]);
		if (it == m_Commands.end()) 
		{
			//AddLog(Stringf("Command not found: %s",parts[0].c_str()));
			return;
		}

		if (parts[0] == "echo")
		{
			std::string message = "";
			for (size_t i = 1; i < parts.size(); i++)
			{
				message += parts[i];
				if (i < parts.size() - 1)
				{
					message += " ";
				}
			}
			std::vector<std::string> args = { message };
			if (!it->second->Execute(args))
			{
				AddLog(Stringf("Failed to execute command: %s", parts[0].c_str()));
			}
		}
		else
		{
			std::vector<std::string> args(parts.begin() + 1, parts.end());
			if (!it->second->Execute(args)) 
			{
				AddLog(Stringf("Failed to execute command: %s", parts[0].c_str()));
			}
		}
	}

	void DevConsole::KeyPressed(int keycode)
	{
		switch (keycode)
		{
			case PBE::KEY_F1:
				if (m_Config.m_pWindow->GetCursorMode() == PBE::CursorMode::NORMAL)
				{
					m_Config.m_pWindow->SetCursorMode(PBE::CursorMode::FPS);
				}
				else
				{
					m_Config.m_pWindow->SetCursorMode(PBE::CursorMode::NORMAL);
				}
				return;
			case GLFW_KEY_ENTER:
				ExecuteCommand(m_InputBuffer);
				m_InputBuffer.clear();
				m_CursorPosition = 0;
				return;
			case GLFW_KEY_LEFT:
				if (m_CursorPosition > 0)
				{
					m_CursorPosition--;
				}
				return;
			case GLFW_KEY_RIGHT:
				if (m_CursorPosition < m_InputBuffer.size())
				{
					m_CursorPosition++;
				}
				return;
			case GLFW_KEY_BACKSPACE:
				if (m_CursorPosition > 0)
				{
					m_InputBuffer.erase(m_InputBuffer.begin() + m_CursorPosition - 1);
					m_CursorPosition--;
				}
				return;
		}
	}
	void DevConsole::KeyReleased(int)
	{
	}
	void DevConsole::KeyTyped(int keycode)
	{
		m_InputBuffer.insert(m_InputBuffer.begin() + m_CursorPosition, (char)keycode);
		m_CursorPosition++;
	}

	void DevConsole::SetupCommands()
	{
		RegisterCommand<std::string>(
			"echo", { [this](std::string message) {
				AddLog(message);
				return true;
				} },
			"Prints a message to the console."
		);

		RegisterCommand("clear", { [this]() {
			m_Logs.clear();
			system("cls");
			return true;
		} }, "Clears the console screen.");

		RegisterCommand("exit", { [this]() {
			m_Config.m_pWindow->SetWindowShouldClose(true);
			return true;
		} }, "Exits the application.");
	}
}

