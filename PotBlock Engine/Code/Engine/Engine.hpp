#pragma once
#include "Audio/AudioSystem.hpp"

#include "Engine/Graphics/Window.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/BitmapFontSystem.hpp"
#include "Engine/Graphics/Texture.hpp"
#include "Engine/Graphics/TextureAtlas.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/UI/Theme.hpp"
#include "Engine/Graphics/UI/Canvas.hpp"
#include "Engine/Graphics/UI/DevConsoleGUI.hpp"
#include "Engine/Core/HCIString.hpp"

#include "Engine/Scene/SkeletalModel.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Scripting/LuaSystem.hpp"
#include "Engine/Threads/JobSystem.hpp"
#include "Engine/Time/Clock.hpp"

namespace PBE
{
	struct EngineCreateInfo
	{
		uint32_t m_AppMajorVersion;
		uint32_t m_AppMinorVersion;
		uint32_t m_AppPatchVersion;

		std::string m_AppName;
		std::filesystem::path m_IconPath;

		bool m_EnableDevConsole;
		bool m_EnableAudio;
		bool m_EnableLua;
		bool m_EnableWindow;
		bool m_EnableJobSystem;
		bool m_EnableInputSystem;
		bool m_EnableFontSystem;

		int m_numJobThreads;

		RendererCreateInfo m_RendererInfo
		{
			.ApplicationName = "PBE",
			.EngineName = "PBE"
		};
		WindowCreateInfo m_WindowInfo;
		AudioSystemCreateInfo m_AudioSystemInfo;
		LuaSystemCreateInfo m_LuaSystemInfo;
		InputSystemCreateInfo m_InputSystemInfo;
		BitmapFontSystemCreateInfo m_FontSystemInfo;
	};

	class Engine
	{
	public:
		EngineCreateInfo m_Info;
		bool   m_Running = true;
		Clock* m_pClock					= nullptr;
		DevConsole* m_pConsole			= nullptr;
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Debug UI
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		PBE::Canvas*		m_pDebugCanvas		= nullptr;
		PBE::DevConsoleGUI* m_pDevConsoleGUI	= nullptr;
		PBE::TextPanel*		m_pFPSPanel			= nullptr;

		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Systems
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		Renderer* m_pRenderer			= nullptr;
		Window* m_pWindow				= nullptr;
		AudioSystem* m_pAudioSystem		= nullptr;
		LuaSystem* m_pLuaSystem			= nullptr;
		JobSystem* m_pJobSystem			= nullptr;
		InputSystem* m_pInputSystem		= nullptr;
		BitmapFontSystem* m_pFontSystem	= nullptr;

		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Resources
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Resource reference pointers
		std::weak_ptr<BitmapFont>   m_DefaultFont;
		std::weak_ptr<TextureAtlas> m_SpriteAtlas;

		//Owned Resources
		std::shared_ptr<Theme>		m_DefaultTheme;
		std::shared_ptr<Material>	m_DefaultWhiteMaterial;
		std::shared_ptr<Material>	m_DefaultUIMaterial;

		std::unordered_map<HCIString, std::shared_ptr<Material>>		m_Materials;
		std::unordered_map<HCIString, std::shared_ptr<TextureAtlas>>	m_TextureAtlases;
		std::unordered_map<HCIString, std::shared_ptr<BitmapFont>>		m_Fonts;
		std::unordered_map<HCIString, std::shared_ptr<Sound>>			m_Sounds;
		std::unordered_map<HCIString, std::shared_ptr<SkeletalModel>>	m_SkeletalModels;
		std::unordered_map<HCIString, std::shared_ptr<Theme>>			m_Themes;
	public:
		Engine(EngineCreateInfo const& info);
		~Engine();

		bool IsRunning();
		void Quit();

		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Core
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		void Startup();
		void Shutdown();
		void BeginFrame();
		void EndFrame();

		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		//Resources
		//-----------------------------------------------------------------------------------------------------------------------------------------------------
		std::weak_ptr<TextureAtlas> LoadTextureAtlasFromJSON(std::string_view uniqueName, std::filesystem::path path, SamplerType type);
		std::weak_ptr<TextureAtlas> LoadOrGetTextureAtlas(std::string_view uniqueName, TextureAtlasCreateInfo const& info);
		std::weak_ptr<TextureAtlas> GetTextureAtlas(std::string_view uniqueName);

		std::weak_ptr<Material>     LoadOrGetMaterial(std::string_view uniqueName, MaterialCreateInfo const& info);
		std::weak_ptr<Material>     GetMaterial(std::string_view uniqueName);

		std::weak_ptr<BitmapFont>	LoadOrGetFont(std::string_view uniqueName, BitmapFontCreateInfo const& info);
		std::weak_ptr<BitmapFont>	GetFont(std::string_view uniqueName);

		std::weak_ptr<Sound>		LoadOrGetSound(std::string_view uniqueName, std::filesystem::path path, AudioMode mode);
		std::weak_ptr<Sound>		GetSound(std::string_view uniqueName);

	private:
		void InitImGui();
		void CreateDefaultResources();
	};
}