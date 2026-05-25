#pragma once
#include "Cubikki/Common.hpp"
#include "Cubikki/Game/Game.hpp"

#include "Engine/Graphics/Window.hpp"
#include "Engine/Graphics/BitmapFontSystem.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Camera.hpp"

#include "Engine/Graphics/UI/Canvas.hpp"
#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Graphics/UI/DevConsoleGUI.hpp"
#include "Engine/Graphics/UI/TextPanel.hpp"
#include "Engine/Graphics/UI/ImagePanel.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Scripting/LuaSystem.hpp"

#include <string>

class BlockWorld;

struct ApplicationCreateInfo
{
	std::filesystem::path IconPath;
	std::string ApplicationName;
	std::string EngineName;
};

class App
{
public:
	//BlockWorld* m_pBlockWorld;
public:
	ApplicationCreateInfo m_Info;
public:
	App(ApplicationCreateInfo const& info);
	~App();

	void Startup();

	void BeginFrame();
	void Update();
	void EndFrame();
	void Run();
};