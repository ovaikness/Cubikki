#include "Cubikki/Game/GMClientGame.hpp"
#include "Cubikki/Actors/ActorUtils.hpp"
#include "Cubikki/Game/Game.hpp"

#include "Cubikki/Actors/ActorClasses/APlayerController.hpp"
#include "Cubikki/Actors/ActorClasses/AFlyCam.hpp"
#include "Cubikki/Actors/ActorClasses/ACharacter.hpp"

#include <chrono>
#include <thread>

GMClientGame::GMClientGame(Game* game)
	: GameMode(game)
{

	//PBE::CanvasCreateInfo info{};
	//info.m_pRenderer = g_Engine->m_pRenderer;
	//info.m_pTheme = g_Engine->m_DefaultTheme.get();
	//info.m_pWindow = g_Engine->m_pWindow;
	//
	//m_Canvas = new PBE::Canvas(info);

}

GMClientGame::~GMClientGame()
{
}

PBE::Result GMClientGame::Startup()
{
	return PBE::RESULT_SUCCESS;
}

PBE::Result GMClientGame::Shutdown()
{
	delete m_pBlockWorld;

	return PBE::RESULT_SUCCESS;
}

void GMClientGame::OnGameModeChangedTo()
{
	int tries = 10;
	while (tries > 0)
	{
		if (g_Client->Connect(g_IpAddress, g_Port))
		{
			std::cout << "Connected to server." << std::endl;
			break;
		}
		std::cout << "Connecting..." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		tries--;
	}

	Load("TestWorld");

	REGISTER_CPP_ACTOR(AFlyCam);
	REGISTER_CPP_ACTOR(APlayerController);
	REGISTER_CPP_ACTOR(ACharacter);

	if (tries > 0)
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Header.m_ID = CubikkiMessageType::ServerPing;
		g_Client->Send(msg);
	}
	else
	{
		std::cout << "Failed to connect to server." << std::endl;
		m_Game->SetGameMode(GameModeType::MAIN_MENU);
	}
}

void GMClientGame::OnGameModeChangedFrom()
{
	g_Client->Disconnect();
	delete m_pBlockWorld;
	m_pBlockWorld = nullptr;
}

void GMClientGame::Load(std::string_view worldName)
{
	delete m_pBlockWorld;
	m_pBlockWorld = nullptr;

	m_pBlockWorld = new BlockWorld(worldName, g_Client);
	m_pBlockWorld->m_LocalPlayerName = "PlayerClient";
}

void GMClientGame::Update()
{
	m_pBlockWorld->SetPlayerPosition("PlayerClient", PBE::IntVec3{ m_Game->m_Camera->m_Position });
	m_pBlockWorld->Update();
	if (!g_Client->IsConnected())
	{
		m_Game->SetGameMode(GameModeType::MAIN_MENU);
	}
	if (g_Engine->m_pInputSystem->WasKeyPressed(PBE::KEY_ESCAPE))
	{
		g_Game->SetGameMode(GameModeType::MAIN_MENU);
	}
}

void GMClientGame::CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	m_pBlockWorld->CmdTransfer(renderer, buffer, frameIndex);
}

void GMClientGame::CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_Game->m_Camera)
	{
		m_pBlockWorld->CmdDraw(m_Game->m_Camera, renderer, buffer, frameIndex);
	}
}

void GMClientGame::CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_Game->m_Camera)
	{
		m_pBlockWorld->CmdDrawTranslucent(m_Game->m_Camera, renderer, buffer, frameIndex);
	}
}

void GMClientGame::CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	//Nothing... FOR NOW
}