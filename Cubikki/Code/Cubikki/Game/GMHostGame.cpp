#include "Cubikki/Game/GMHostGame.hpp"
#include "Cubikki/Actors/ActorUtils.hpp"
#include "Cubikki/Game/Game.hpp"

#include "Cubikki/Actors/ActorClasses/APlayerController.hpp"
#include "Cubikki/Actors/ActorClasses/AFlyCam.hpp"
#include "Cubikki/Actors/ActorClasses/ACharacter.hpp"

GMHostGame::GMHostGame(Game* game)
	: GameMode(game)
{

}

GMHostGame::~GMHostGame()
{
}

PBE::Result GMHostGame::Startup()
{
	return PBE::RESULT_SUCCESS;
}

void GMHostGame::OnGameModeChangedTo()
{
	Load("TestWorld");

	PBE::NamedProperties props;
	props["Position"] = PBE::Vec3(0.f, 0.f, 0.f);
	props["ClientID"] = (int)SERVER_CLIENT_ID;
	REGISTER_CPP_ACTOR(AFlyCam);
	REGISTER_CPP_ACTOR(APlayerController);
	REGISTER_CPP_ACTOR(ACharacter);
	RegisterLuaActor("TestActor", "ACharacter", { "Test.lua" });

	ActorUID flyCam = m_pBlockWorld->SpawnActor("AFlyCam", props);
	ActorUID controller = m_pBlockWorld->SpawnActor("APlayerController", props);
	APlayerController* controllerActor = m_pBlockWorld->GetActor<APlayerController>(controller);

	controllerActor->Possess(flyCam);

	g_Server->Start();
}

void GMHostGame::OnGameModeChangedFrom()
{
	g_Server->Stop();
	delete m_pBlockWorld;
	m_pBlockWorld = nullptr;
}

PBE::Result GMHostGame::Shutdown()
{
	delete m_pBlockWorld;
	m_pBlockWorld = nullptr;

	return PBE::RESULT_SUCCESS;
}

void GMHostGame::Load(std::string_view worldName)
{
	delete m_pBlockWorld;
	m_pBlockWorld = nullptr;

	m_pBlockWorld = new BlockWorld(worldName, g_Server);
	m_pBlockWorld->m_LocalPlayerName = "PlayerServer";
}

void GMHostGame::Update()
{
	m_pBlockWorld->SetPlayerPosition("PlayerServer", PBE::IntVec3{ m_Game->m_Camera->m_Position });
	m_pBlockWorld->Update();

	if (g_Engine->m_pInputSystem->WasKeyPressed(PBE::KEY_ESCAPE))
	{
		g_Game->SetGameMode(GameModeType::MAIN_MENU);
	}
}

void GMHostGame::CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	m_pBlockWorld->CmdTransfer(renderer, buffer, frameIndex);
}

void GMHostGame::CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_Game->m_Camera)
	{
		m_pBlockWorld->CmdDraw(m_Game->m_Camera, renderer, buffer, frameIndex);
	}
}

void GMHostGame::CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_Game->m_Camera)
	{
		m_pBlockWorld->CmdDrawTranslucent(m_Game->m_Camera, renderer, buffer, frameIndex);
	}
}

void GMHostGame::CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	//Nothing... FOR NOW
}