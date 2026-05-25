#include "Cubikki/Game/GMMenu.hpp"
#include "Cubikki/Game/Game.hpp"

#include "Engine/Math/MathUtils.hpp"

GMMenu::GMMenu(Game* game)
	: GameMode(game)
{
}

GMMenu::~GMMenu()
{

}

PBE::Result GMMenu::Startup()
{
	m_CanvasTimer = PBE::Timer(nullptr, 0.016f);
	m_CanvasTimer.Start();

	m_pTextureAtlas = g_Engine->LoadTextureAtlasFromJSON("Menu Atlas", "Assets/MenuAtlas.json", PBE::SamplerType::LINEAR_CLAMP);

	PBE::MaterialCreateInfo materialInfo;

	materialInfo.m_DiffuseTexture = m_pTextureAtlas.lock()->GetTexture();
	materialInfo.m_SpecularGlossEmitTexture = m_pTextureAtlas.lock()->GetTexture();
	materialInfo.m_NormalTexture = m_pTextureAtlas.lock()->GetTexture();
	materialInfo.m_ParallaxTexture = m_pTextureAtlas.lock()->GetTexture();

	materialInfo.m_Device = g_Engine->m_pRenderer->m_Device;
	materialInfo.m_Allocator = g_Engine->m_pRenderer->m_Allocator;

	materialInfo.m_MaterialConstants.m_DiffuseColor = PBE::Rgba8::WHITE.ToVec4();
	materialInfo.m_MaterialConstants.m_SpecularColor = PBE::Rgba8::WHITE.ToVec4();
	materialInfo.m_MaterialConstants.m_AmbientColor = PBE::Rgba8::WHITE.ToVec4();
	materialInfo.m_MaterialConstants.m_Shininess = 0.f;

	m_pMaterial = g_Engine->m_pRenderer->CreateMaterialShared(materialInfo);

	PBE::CanvasCreateInfo canvasInfo;
	canvasInfo.m_pRenderer = g_Engine->m_pRenderer;
	canvasInfo.m_pTheme = g_Engine->m_DefaultTheme.get();
	canvasInfo.m_pWindow = g_Engine->m_pWindow;

	m_pMainCanvas = new PBE::Canvas(canvasInfo);

	m_pJoinCanvas = new PBE::VerticalContainerWidget();
	m_pJoinCanvas->m_NormalizedAnchors = PBE::AABB2(0.25f,0.f,1.f,1.f);
	m_pJoinCanvas->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 0.f, -120.f);

	PBE::Vec2 size;
	m_pBackgroundPanel = new PBE::ImagePanel();
	m_pBackgroundPanel->m_MaterialOverride = m_pMaterial;
	m_pBackgroundPanel->m_UVCoords = m_pTextureAtlas.lock()->GetTextureRegion("background.png").m_UV;
	size = m_pTextureAtlas.lock()->GetTextureRegion("background.png").m_Size;
	m_pBackgroundPanel->m_NormalizedAnchors = PBE::AABB2(0.5f, 0.5f, 0.5f, 0.5f);
	m_pBackgroundPanel->m_AnchorOffsetInPixels = PBE::AABB2(-size.x, -size.y, size.x, size.y);

	m_pTitlePanel = new PBE::ImagePanel();
	m_pTitlePanel->m_MaterialOverride = m_pMaterial;
	m_pTitlePanel->m_UVCoords = m_pTextureAtlas.lock()->GetTextureRegion("cubikki.png").m_UV;
	size = m_pTextureAtlas.lock()->GetTextureRegion("cubikki.png").m_Size;
	m_pTitlePanel->m_NormalizedAnchors = PBE::AABB2(0.f, 0.f, 0.f, 0.f);
	m_pTitlePanel->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 300.f, 300.f);


	m_pNetCanvas = new PBE::VerticalContainerWidget();
	m_pNetCanvas->m_NormalizedAnchors = PBE::AABB2(0.25f, 0.f, 1.f, 1.f);
	m_pNetCanvas->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, -80.f, -80.f);
	m_pNetCanvas->m_UseHeightOverride = false;
	m_pNetCanvas->m_UseOffsetHeight = true;

	m_pMainMenuContainer = new PBE::VerticalContainerWidget();
	m_pMainMenuContainer->m_NormalizedAnchors = PBE::AABB2(0.f,0.f,0.15f,1.f);
	m_pMainMenuContainer->m_AnchorOffsetInPixels = PBE::AABB2(80.f, 0.f, 80.f, 0.f);
	m_pMainMenuContainer->m_UseOffsetHeight = true;
	m_pMainMenuContainer->m_UseOffsetWidth = true;
	m_pMainMenuContainer->m_UseHeightOverride = false;

	//Main Menu Buttons
	m_pPlayButton = new PBE::TextPanel("Play", PBE::Vec2(0.f, 0.5f), PBE::Rgba8::WHITE);
	
	m_pPlayButtonHoverBox = new PBE::Widget();
	m_pPlayButtonHoverBox->m_IsHoverable = true;
	m_pPlayButtonHoverBox->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 300.f, 84.f);

	m_pPlayButtonHoverBox->AddChild(m_pPlayButton);

	m_pMultiplayerButton = new PBE::TextPanel("Multiplayer", PBE::Vec2(0.f, 0.5f), PBE::Rgba8::WHITE);

	m_pMultiplayerButtonHoverBox = new PBE::Widget();
	m_pMultiplayerButtonHoverBox->m_IsHoverable = true;
	m_pMultiplayerButtonHoverBox->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 300.f, 84.f);

	m_pMultiplayerButtonHoverBox->AddChild(m_pMultiplayerButton);

	m_pOptionsButton = new PBE::TextPanel("Options", PBE::Vec2(0.f, 0.5f), PBE::Rgba8::WHITE);
	
	m_pOptionsButtonHoverBox = new PBE::Widget();
	m_pOptionsButtonHoverBox->m_IsHoverable = true;
	m_pOptionsButtonHoverBox->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 300.f, 84.f);

	m_pOptionsButtonHoverBox->AddChild(m_pOptionsButton);

	m_pQuitButton = new PBE::TextPanel("Quit", PBE::Vec2(0.f, 0.5f), PBE::Rgba8::WHITE);
	
	m_pQuitButtonHoverBox = new PBE::Widget();
	m_pQuitButtonHoverBox->m_IsHoverable = true;
	m_pQuitButtonHoverBox->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 300.f, 84.f);

	m_pQuitButtonHoverBox->AddChild(m_pQuitButton);

	auto hoverUpdate = [this](PBE::Widget* hovered, PBE::Widget* widget)
		{
			float dt = PBE::Clock::GetSystemClock()->GetDeltaSeconds();
			auto& props = widget->m_Properties;
			float t;
			if (props.GetFloat("transition", t) != PBE::RESULT_SUCCESS)
			{
				props["transition"] = 0.f;
				t = 0.f;
			}
			if (hovered->m_IsHovered)
			{
				t += 3.f * dt;
			}
			else
			{
				t -= 3.f * dt;
			}
			t = PBE::Clamp(t, 0.f, 1.f);
			props.SetFloat("transition", t);
			float smoothT = PBE::SmoothStop5(0.f, 1.f, t);
			widget->m_Color = PBE::Rgba8::Lerp(PBE::Rgba8::WHITE, PBE::Rgba8::YELLOW, smoothT);
			float offset = smoothT * 10.f;
			widget->m_AnchorOffsetInPixels = PBE::AABB2(offset, 0.f, offset,0.f);
			widget->UpdateTransform();
			widget->MarkDirty();
			return false;
		};

	m_pPlayButtonHoverBox->m_OnUpdate.AddSubscriber([this, &hoverUpdate]()
		{
			hoverUpdate(m_pPlayButtonHoverBox, m_pPlayButton);
			return false;
		}
	);

	m_pMultiplayerButtonHoverBox->m_OnUpdate.AddSubscriber([this, &hoverUpdate]()
		{
			hoverUpdate(m_pMultiplayerButtonHoverBox, m_pMultiplayerButton);
			return false;
		}
	);

	m_pQuitButtonHoverBox->m_OnUpdate.AddSubscriber([this, &hoverUpdate]()
		{
			hoverUpdate(m_pQuitButtonHoverBox, m_pQuitButton);
			return false;
		}
	);

	m_pOptionsButtonHoverBox->m_OnUpdate.AddSubscriber([this, &hoverUpdate]()
		{
			hoverUpdate(m_pOptionsButtonHoverBox, m_pOptionsButton);
			return false;
		}
	);

	m_pQuitButtonHoverBox->m_OnActivated.AddSubscriber([this]()
		{
			g_Engine->Quit();
			return false;
		});

	m_pPlayButtonHoverBox->m_OnActivated.AddSubscriber([this]()
		{
			m_Game->SetGameMode(GameModeType::GAMEPLAY);
			return false;
		}
	);

	m_pMultiplayerButtonHoverBox->m_OnActivated.AddSubscriber([this]()
		{
			//m_MenuState = MenuState::JOIN;
			m_Game->SetGameMode(GameModeType::CLIENT_GAME);
			return false;
		}
	);

	m_pOptionsButtonHoverBox->m_OnActivated.AddSubscriber([this]()
		{
			m_MenuState = MenuState::NET;
			return false;
		}
	);

	auto activatedCallback = [this]()
		{
			PBE::SoundPlaybackConfig config;
			config.m_looping = false;
			config.m_paused = false;
			config.m_volume = 0.5f;
			config.m_pitch = 2.f;

			g_Engine->m_pAudioSystem->PlaySound2D(g_Engine->LoadOrGetSound("Menu Activated", "Assets/Audio/menu_button_activated.mp3", PBE::AUDIO_MODE_2D).lock().get(), config, "Sound");
			return false;
		};
	auto hoveredCallback = [this]()
		{
			PBE::SoundPlaybackConfig config;
			config.m_looping = false;
			config.m_paused = false;
			config.m_volume = 0.5f;
			config.m_pitch = 2.5f;

			g_Engine->m_pAudioSystem->PlaySound2D(g_Engine->LoadOrGetSound("Menu Hovered", "Assets/Audio/menu_button_hover.mp3", PBE::AUDIO_MODE_2D).lock().get(), config, "Sound");

			return false;
		};
	m_pPlayButtonHoverBox->m_OnActivated.AddSubscriber(activatedCallback);
	m_pMultiplayerButtonHoverBox->m_OnActivated.AddSubscriber(activatedCallback);
	m_pOptionsButtonHoverBox->m_OnActivated.AddSubscriber(activatedCallback);
	m_pQuitButtonHoverBox->m_OnActivated.AddSubscriber(activatedCallback);
	m_pPlayButtonHoverBox->m_OnHovered.AddSubscriber(hoveredCallback);
	m_pMultiplayerButtonHoverBox->m_OnHovered.AddSubscriber(hoveredCallback);
	m_pOptionsButtonHoverBox->m_OnHovered.AddSubscriber(hoveredCallback);
	m_pQuitButtonHoverBox->m_OnHovered.AddSubscriber(hoveredCallback);

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//Net Menu
//-----------------------------------------------------------------------------------------------------------------------------------------------------
	m_pHostButton = new PBE::TextPanel("IP Address", PBE::Vec2(0.f, 0.5f), PBE::Rgba8::WHITE);
	m_pHostButton->m_FontScale = 0.5f;
	m_pHostButton->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 0.f, 20.f);
	m_pIpTextEntry = new PBE::TextEntryPanel(g_Engine->m_pWindow);
	m_pIpTextEntry->m_AnchorOffsetInPixels = PBE::AABB2(0.f, 0.f, 0.f, 42.f);
	m_pIpTextEntry->m_OnKeyTyped.AddSubscriber([this](int key)
		{
			g_Engine->m_pAudioSystem->PlaySound2D(g_Engine->LoadOrGetSound("Key Typed", "Assets/Audio/key_typed.mp3", PBE::AUDIO_MODE_2D).lock().get(), {}, "Sound");
			return false;
		});

	m_pNetCanvas->AddChild(m_pHostButton);
	m_pNetCanvas->AddChild(m_pIpTextEntry);

	m_pMainCanvas->AddChild(m_pBackgroundPanel);
	//m_pMainCanvas->AddChild(m_pTitlePanel);
	m_pMainCanvas->AddChild(m_pMainMenuContainer);

	m_pMainMenuContainer->AddChild(m_pTitlePanel);
	m_pMainMenuContainer->AddChild(m_pPlayButtonHoverBox);
	m_pMainMenuContainer->AddChild(m_pMultiplayerButtonHoverBox);
	m_pMainMenuContainer->AddChild(m_pOptionsButtonHoverBox);
	m_pMainMenuContainer->AddChild(m_pQuitButtonHoverBox);

	m_pMainCanvas->AddChild(m_pNetCanvas);

	m_pMainCanvas->UpdateTransform();

	g_Engine->m_pWindow->m_OnResize.AddSubscriber([this](int width, int height)
		{
			m_pMainCanvas->OnResize(width, height);
			return false;
		});

	return PBE::RESULT_SUCCESS;
}

PBE::Result GMMenu::Shutdown()
{
	delete m_pMainCanvas;
	delete m_pJoinCanvas;
	delete m_pNetCanvas;

	delete m_pPlayButton;

	return PBE::RESULT_SUCCESS;
}


void GMMenu::Update()
{
	g_Engine->m_pWindow->SetCursorMode(PBE::CursorMode::NORMAL);
	m_pMainCanvas->UpdateCursorPos(g_Engine->m_pWindow->GetCursorPos());

	if (g_Engine->m_pInputSystem->IsMouseButtonPressed(PBE::MOUSE_BUTTON_1))
	{
		m_pMainCanvas->OnActivated();
	}

	m_pMainCanvas->Update();

	if (g_Engine->m_pInputSystem->WasKeyPressed(PBE::KEY_ESCAPE))
	{
		g_Engine->Quit();
	}

	std::string fullIp = m_pIpTextEntry->m_Text;
	if (fullIp.find(':') != std::string::npos)
	{
		std::string ip = fullIp.substr(0, fullIp.find(':'));
		std::string port = fullIp.substr(fullIp.find(':') + 1);
		//if (g_Engine->m_pInputSystem->WasKeyPressed(PBE::KEY_ENTER))
		//{
		if (port.length() > 0)
		{
			g_IpAddress = ip;
			g_Port = std::stoi(port);
		}
		//	m_Game->SetGameMode(GameModeType::CLIENT_GAME);
		//}
	}
}

void GMMenu::CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	m_pMainCanvas->CmdTransfer(renderer, buffer, frameIndex);
}

void GMMenu::CmdDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
}

void GMMenu::CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
}

void GMMenu::CmdPostProcessDraw(PBE::Renderer* renderer, VkCommandBuffer buffer, uint32_t frameIndex)
{
	m_pMainCanvas->CmdPostProcessDraw(buffer, g_Engine->m_pRenderer->GetUIPipelineLayout(), frameIndex);
}
