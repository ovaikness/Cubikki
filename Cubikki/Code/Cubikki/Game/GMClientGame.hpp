#pragma once

#include "Cubikki/Game/GameMode.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"

#include "Engine/Graphics/Camera.hpp"
#include "Engine/Graphics/UI/Canvas.hpp"
#include "Engine/Graphics/UI/DevConsoleGUI.hpp"

class BlockWorld;

#include <filesystem>
#include <iostream>

class GMClientGame : public GameMode
{
public:
	BlockWorld* m_pBlockWorld{ nullptr };
	PBE::Canvas* m_Canvas{ nullptr };
	PBE::DevConsoleGUI* m_DevConsole{ nullptr };
public:
	GMClientGame(Game* game);
	~GMClientGame();
	virtual PBE::Result Startup() override;
	virtual PBE::Result Shutdown() override;
	virtual void OnGameModeChangedTo() override;
	virtual void OnGameModeChangedFrom() override;
	virtual void Load(std::string_view worldName);
	virtual void Update() override;
	virtual void CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex) override;
};