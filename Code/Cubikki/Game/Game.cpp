#include "Cubikki/Game/Game.hpp"

#include "Cubikki/Game/GameMode.hpp"
#include "Cubikki/Game/GMMenu.hpp"
#include "Cubikki/Game/GMHostGame.hpp"
#include "Cubikki/Game/GMClientGame.hpp"

#include "Cubikki/Common.hpp"

std::string g_IpAddress;
uint16_t g_Port;

Game::Game()
{
}

Game::~Game()
{
}

PBE::Result Game::Startup()
{
	m_SkeletalModelSystem = new SkeletalModelSystem(g_Engine->m_pRenderer);
	m_TransformSystem = new TransformSystem();
	m_Camera = g_Engine->m_pRenderer->CreateCamera();
	m_Camera->CreatePerspective(90.f, g_Engine->m_pWindow->GetAspectRatio(), 0.1f, 300.f);
	m_Camera->m_ViewportMode = PBE::ViewportMode::STRETCH;
	m_Camera->OnResize(g_Engine->m_pWindow->GetWidth(), g_Engine->m_pWindow->GetHeight());
	m_Camera->SetLocalPosition(PBE::Vec3(0.f, 0.f, 0.f));
	m_Camera->SetOrientation(PBE::EulerAngles(0.f, 0.f, 0.f));
	
	g_Engine->m_pWindow->m_OnResize.AddSubscriber([this](int width, int height)
		{
			m_Camera->OnResize(width, height);
			return false;
		});

	PBE::Result result = InitializeGamemodes();

	if (result != 0)
	{
		return result;
	}

	return PBE::RESULT_SUCCESS;
}

PBE::Result Game::Shutdown()
{
	delete m_Camera;
	delete m_SkeletalModelSystem;
	delete m_TransformSystem;
	for (auto& gameMode : m_GameModes)
	{
		if (!gameMode)
		{
			continue;
		}
		PBE::Result result;
		result = gameMode->Shutdown();
		if (result != PBE::RESULT_SUCCESS)
		{
			return result;
		}
		delete gameMode;
	}
	return PBE::RESULT_SUCCESS;
}

PBE::Result Game::InitializeGamemodes()
{
	m_GameModes.resize((size_t)GameModeType::COUNT);

	m_GameModes[(size_t)GameModeType::MAIN_MENU] = (GameMode*)new GMMenu(this);
	m_GameModes[(size_t)GameModeType::GAMEPLAY] = (GameMode*)new GMHostGame(this);
	m_GameModes[(size_t)GameModeType::CLIENT_GAME] = (GameMode*)new GMClientGame(this);
	m_CurrentGameMode = m_GameModes[(int)m_GameModeType];
	m_CurrentGameMode->m_IsEnabled = true;

	for (auto& gameMode : m_GameModes)
	{
		if (!gameMode)
		{
			continue;
		}

		PBE::Result result;
		result = gameMode->Startup();
		if (result != 0)
		{
			return result;
		}
	}

	return PBE::RESULT_SUCCESS;
}

void Game::SetGameMode(GameModeType gameModeType)
{
	m_GameModeTypeNextFrame = gameModeType;
}

void Game::Update()
{
	bool changed = m_GameModeType != m_GameModeTypeNextFrame;
	m_GameModeType = m_GameModeTypeNextFrame;
	m_CurrentGameMode->m_IsEnabled = false;

	if (changed)
	{
		m_CurrentGameMode->OnGameModeChangedFrom();
	}

	m_CurrentGameMode = m_GameModes[(int)m_GameModeType];
	m_CurrentGameMode->m_IsEnabled = true;

	if (changed)
	{
		m_CurrentGameMode->OnGameModeChangedTo();
	}

	if (m_CurrentGameMode)
	{
		m_CurrentGameMode->Update();
		m_TransformSystem->Update();
	}

	if (g_Engine->m_pInputSystem->WasKeyPressed(PBE::KEY_F11))
	{
		g_Engine->m_pWindow->SetFullscreen(!g_Engine->m_pWindow->GetFullscreen());
	}
}

void Game::CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	m_Camera->CmdTransfer(buffer, frameIndex);
	if (m_CurrentGameMode)
	{
		m_CurrentGameMode->CmdTransfer(renderer, buffer, frameIndex);
	}
}

void Game::CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_CurrentGameMode)
	{
		m_CurrentGameMode->CmdDraw(renderer, buffer, frameIndex);
		m_SkeletalModelSystem->CmdDraw(m_Camera, buffer, renderer->GetHDRPipelineLayout(), frameIndex);
	}
}

void Game::CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_CurrentGameMode)
	{
		m_CurrentGameMode->CmdDrawTranslucent(renderer, buffer, frameIndex);
		//m_SkeletalModelSystem->CmdDraw(m_Camera, buffer, renderer->GetHDRPipelineLayout(), frameIndex);
	}
}


void Game::CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	if (m_CurrentGameMode)
	{
		m_CurrentGameMode->CmdPostProcessDraw(renderer, buffer, frameIndex);
	}
}
