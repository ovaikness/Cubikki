#pragma once

#include "Engine/Input/KeyCodes.hpp"
#include "Engine/IO/FileUtils.hpp"
#include "Engine/Core/Result.hpp"

#include "Engine/Graphics/Window.hpp"

#include "Engine/Core/Command.hpp"

#include <unordered_map>
#include <mutex>

namespace PBE
{
	struct DevConsoleConfig
	{
		PBE::Window* m_pWindow = nullptr;
	};

	class DevConsole
	{
	public:
		bool m_Visible = false;

		int m_CursorPosition = 0;
		std::unordered_map<std::string, std::shared_ptr<CommandBase>> m_Commands;

		std::mutex m_LogMutex;
		std::vector<std::string> m_Logs;
		std::string m_InputBuffer;
	public:
		DevConsole(DevConsoleConfig const& config);
		~DevConsole();

		template<typename... Args>
		void RegisterCommand(std::string_view name, std::function<bool(Args...)> func, std::string_view description)
		{
			m_Commands[std::string(name)] = std::make_shared<Command<Args...>>(func, description);
		}

		void AddLog(std::string_view message);
		void AddWarning(std::string_view message);

		void ExecuteCommand(std::string_view const command);
		void KeyPressed(int keycode);
		void KeyReleased(int keycode);
		void KeyTyped(int keycode);

	protected:
		DevConsoleConfig m_Config;
	protected:
		void SetupCommands();
	};
}