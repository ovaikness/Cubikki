#pragma once
#include "Cubikki/Game/GameMode.hpp"

#include "Engine/Graphics/UI/Canvas.hpp"
#include "Engine/Graphics/UI/TextPanel.hpp"
#include "Engine/Graphics/UI/TextEntryPanel.hpp"
#include "Engine/Graphics/UI/ImagePanel.hpp"
#include "Engine/Graphics/UI/VerticalContainerWidget.hpp"

#include "Engine/Time/Timer.hpp"

#include "Engine/Graphics/TextureAtlas.hpp"

#include <iostream>
#include <vector>

enum MenuState
{
	MAIN,
	NET,
	OPTIONS,
	JOIN
};

class GMMenu : public GameMode
{
public:
	std::shared_ptr<PBE::Material> m_pMaterial;
	std::weak_ptr<PBE::TextureAtlas> m_pTextureAtlas;

	PBE::Timer m_CanvasTimer;

	PBE::Canvas* m_pMainCanvas				{ nullptr };
	PBE::ImagePanel* m_pBackgroundPanel		{ nullptr };
	PBE::ImagePanel* m_pTitlePanel			{ nullptr };
	PBE::VerticalContainerWidget* m_pMainMenuContainer{ nullptr };
	PBE::Widget* m_pPlayButtonHoverBox		{ nullptr };
	PBE::TextPanel* m_pPlayButton			{ nullptr };

	PBE::Widget* m_pMultiplayerButtonHoverBox{ nullptr };
	PBE::TextPanel* m_pMultiplayerButton	{ nullptr };

	PBE::Widget* m_pOptionsButtonHoverBox	{ nullptr };
	PBE::TextPanel* m_pOptionsButton		{ nullptr };

	PBE::Widget* m_pQuitButtonHoverBox		{ nullptr };
	PBE::TextPanel* m_pQuitButton			{ nullptr };
    
	PBE::VerticalContainerWidget* m_pNetCanvas				{ nullptr };
	PBE::TextPanel* m_pHostButton			{ nullptr };
	PBE::TextPanel* m_pJoinButton			{ nullptr };

	PBE::Widget* m_pJoinCanvas				{ nullptr };
	PBE::TextEntryPanel* m_pIpTextEntry	{ nullptr };
	PBE::TextEntryPanel* m_pJoinPortButton	{ nullptr };
	PBE::TextPanel* m_pJoinBackButton	    { nullptr };
	PBE::TextPanel* m_pJoinConnectButton    { nullptr };

	MenuState m_MenuState = MAIN;

public:
	GMMenu(Game* game);
	~GMMenu();
	virtual PBE::Result Startup() override;
	virtual PBE::Result Shutdown() override;
	virtual void Update() override;

	virtual void CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
	virtual void CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex) override;
};