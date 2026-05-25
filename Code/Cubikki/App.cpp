#include "Cubikki/App.hpp"

#include "Engine/Time/Timer.hpp"
#include "Engine/Time/Clock.hpp"
#include "Cubikki/Net/CubikkiClient.hpp"
#include "Cubikki/Net/CubikkiServer.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"
#include "Cubikki/Blocks/Physics/BlockPhysicsWorld.hpp"
#include "Cubikki/Game/Game.hpp"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "nlohmann/json.hpp"

#include <locale>
#include <codecvt>
#include <string>
#include <iostream>
#include <typeinfo>

PBE::Engine* g_Engine;

bool g_ShowDebugInfo;
CubikkiClient* g_Client;
CubikkiServer* g_Server;
Game* g_Game;

std::weak_ptr<PBE::TextureAtlas> g_BlockAtlas;

App::App(ApplicationCreateInfo const& info)
	: m_Info(info)
{
}

App::~App()
{
	g_Game->Shutdown();
	delete g_Game;
	delete g_Client;
	delete g_Server;
	g_Engine->Shutdown();
	delete g_Engine;
}

void App::Startup()
{
	using namespace PBE;
	using namespace nlohmann;

	g_ShowDebugInfo = false;

	std::ifstream file("Assets/AppConfig.json");
	json j = json::parse(file);

	EngineCreateInfo engineInfo;
	engineInfo.m_AppMajorVersion = j["App Major"];
	engineInfo.m_AppMinorVersion = j["App Minor"];
	engineInfo.m_AppPatchVersion = j["App Patch"];

	engineInfo.m_AppName = j["App Name"];
	engineInfo.m_IconPath = std::filesystem::path( (std::string)j["Icon Path"] );

	engineInfo.m_EnableDevConsole = true;
	engineInfo.m_EnableAudio = true;
	engineInfo.m_EnableLua = true;
	engineInfo.m_EnableWindow = true;
	engineInfo.m_EnableJobSystem = true;
	engineInfo.m_EnableInputSystem = true;
	engineInfo.m_EnableFontSystem = true;

	engineInfo.m_numJobThreads				= std::thread::hardware_concurrency() - 1;
	engineInfo.m_LuaSystemInfo.m_ScriptPath = "Assets/Scripts";
	engineInfo.m_WindowInfo.m_Width			= j["Window"]["Width"];
	engineInfo.m_WindowInfo.m_Height		= j["Window"]["Height"];
	engineInfo.m_WindowInfo.m_Title			= j["Window"]["Title"];
	engineInfo.m_WindowInfo.m_Fullscreen	= j["Window"]["Fullscreen"];
	engineInfo.m_WindowInfo.m_Resizable		= j["Window"]["Resizable"];

	g_Engine = new Engine(engineInfo);
	g_Engine->Startup();

	g_Server = new CubikkiServer(10088);

	g_Client = new CubikkiClient();

	g_BlockAtlas = g_Engine->LoadTextureAtlasFromJSON("Block Atlas", "Assets/BlockAtlas.json", SamplerType::NEAREST_CLAMP);

	g_Game = new Game();
	g_Game->Startup();

	g_Engine->m_pRenderer->m_TransferEvent.AddSubscriber([this](Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
		{
			g_Game->CmdTransfer(renderer, cmd, frameIndex);
			return false;
		});

	g_Engine->m_pRenderer->m_DrawEvent.AddSubscriber([this](Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
		{
			g_Game->CmdDraw(renderer, cmd, frameIndex);
			return false;
		});

	g_Engine->m_pRenderer->m_TranslucentDrawEvent.AddSubscriber([this](Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
		{
			g_Game->CmdDrawTranslucent(renderer, cmd, frameIndex);
			return false;
		}
	);

	g_Engine->m_pRenderer->m_PostProcessEvent.AddSubscriber([this](Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
		{
			g_Game->CmdPostProcessDraw(renderer, cmd, frameIndex);
			return false;
		});
}

void App::BeginFrame()
{
	g_Engine->BeginFrame();

	if (!g_ShowDebugInfo)
	{
		return;
	}

	ImGui::DockSpaceOverViewport(0U, 0, ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::Begin("Frame Time");
	ImGui::Text("Frame Time: %f", PBE::Clock::GetSystemClock()->GetDeltaSeconds());

	const int NUM_SAMPLES = 120; // ~2 seconds at 60 FPS
	static float deltaTimes[NUM_SAMPLES] = {};
	static int currentIndex = 0;

	deltaTimes[currentIndex] = PBE::Clock::GetSystemClock()->GetDeltaSeconds();
	currentIndex = (currentIndex + 1) % NUM_SAMPLES;
	
	ImGui::PlotLines("Frame Time", deltaTimes, NUM_SAMPLES, currentIndex, "Frame Time", 0.0f, 0.1f, ImVec2(0, 100));
	ImGui::End();
}

void App::Update()
{
	g_Server->Update();
	g_Client->Update();
	g_Game->Update();
}

void App::EndFrame()
{
	g_Engine->EndFrame();
}

void App::Run()
{
	while (g_Engine->IsRunning())
	{
		BeginFrame();
		Update();
		EndFrame();
	}
}
