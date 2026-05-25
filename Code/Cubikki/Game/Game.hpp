#pragma once
#include "Cubikki/Game/GameMode.hpp"
#include "Cubikki/Actors/Component/SkeletalModelComponent.hpp"
#include "Cubikki/Actors/Component/TransformComponent.hpp"

#include "Engine/Graphics/Camera.hpp"
#include "Engine/Graphics/Renderer.hpp"

#include "Engine/Core/Result.hpp"

#include <vector>

class Game
{
public:
	PBE::Camera* m_Camera						{ nullptr };

	GameModeType m_GameModeTypeNextFrame		{ GameModeType::MAIN_MENU };
	GameMode* m_CurrentGameMode					{ nullptr };

	SkeletalModelSystem* m_SkeletalModelSystem	{ nullptr };
	TransformSystem*	 m_TransformSystem		{ nullptr };

	std::vector<GameMode*> m_GameModes;
public:
	Game();
	~Game();

	PBE::Result Startup();
	PBE::Result Shutdown();

	PBE::Result InitializeGamemodes();
	void SetGameMode(GameModeType gameModeType);
	void Update();

	void CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex);
	void CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex);
	void CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex);
	void CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex);
private:
	GameModeType m_GameModeType{ GameModeType::MAIN_MENU };
};