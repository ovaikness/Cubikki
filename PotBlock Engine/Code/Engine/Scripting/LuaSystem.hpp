#pragma once

#include "Engine/Core/EventDispatcher.hpp"
#include "Engine/Core/Result.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include <sol/sol.hpp>

#include <string>
#include <map>
#include <vector>
#include <filesystem>

namespace PBE
{
	using ScriptID = size_t;
	using ExposedVariableList = std::vector<std::string>;

	struct LuaSystemCreateInfo
	{
		std::filesystem::path m_ScriptPath;
	};

	class LuaSystem
	{
	private:
		ScriptID m_ScriptIDCounter = 0;
	public:
		LuaSystemCreateInfo m_Info;
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Lua Members
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		sol::state m_Lua;
		std::filesystem::path m_ScriptPath;
		std::vector<sol::protected_function> m_LoadedScripts;
		std::vector<ExposedVariableList> m_ExposedVariables;
		std::vector<EventDispatcher<>> m_ScriptReloadDispatchers;
		std::map<std::string, ScriptID> m_ScriptIDs;

		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Callback Events
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		EventDispatcher<> m_OnUpdate;
		EventDispatcher<Renderer, uint32_t, uint32_t> m_OnCmdTransfer;
		EventDispatcher<Renderer, uint32_t, uint32_t> m_OnCmdDraw;
		EventDispatcher<Renderer, uint32_t, uint32_t> m_OnCmdPostProcessDraw;
	public:
		LuaSystem(LuaSystemCreateInfo const& info);
		~LuaSystem();

		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Global Environment and Script Setup
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		sol::protected_function const& GetScript(ScriptID id);
		EventDispatcher<>& GetScriptReloadDispatcher(ScriptID id);
		ScriptID GetScriptID(std::string_view scriptName);
		sol::environment CreateEnvironment();
		
		void ReloadScript(std::string_view scriptName);
		void ReloadAllScripts();

		void RegisterUserTypes();
		void RegisterLuaStateFunctions();
	};
}