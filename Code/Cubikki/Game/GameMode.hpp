#pragma once

#include "Engine/Core/Result.hpp"
#include "Engine/Graphics/Renderer.hpp"

class Game;

enum class GameModeType : size_t
{
	MAIN_MENU,
	GAMEPLAY,
	CLIENT_GAME,
	COUNT
};

class GameMode
{
public:
	bool m_IsEnabled{ false };
	Game* m_Game;
public:
	GameMode(Game* game)
		: m_Game(game)
	{
	};
	virtual ~GameMode()
	{
	};

	virtual PBE::Result Startup() = 0;
	virtual PBE::Result Shutdown() = 0;
	virtual void Update() = 0;
	virtual void OnGameModeChangedTo() {};
	virtual void OnGameModeChangedFrom() {};
	virtual void CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex) = 0;
	virtual void CmdDraw(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex) = 0;
	virtual void CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex) = 0;
	virtual void CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex) = 0;
};