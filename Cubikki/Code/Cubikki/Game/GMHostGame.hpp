#pragma once

#include "Cubikki/Net/CubikkiServer.hpp"
#include "Cubikki/Game/GameMode.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"

#include "Engine/Graphics/Camera.hpp"

class BlockWorld;

#include <filesystem>
#include <iostream>

class GMHostGame : public GameMode
{
public:
	BlockWorld* m_pBlockWorld { nullptr };
public:
	GMHostGame(Game* game);
	~GMHostGame();
	virtual PBE::Result Startup() override;
	virtual void OnGameModeChangedTo() override;
	virtual void OnGameModeChangedFrom() override;
	virtual PBE::Result Shutdown() override;
	virtual void Load(std::string_view worldName);
	virtual void Update() override;
	virtual void CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex) override;
};