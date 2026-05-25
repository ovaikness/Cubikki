#include "Engine/Engine.hpp"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

PBE::Engine::Engine(EngineCreateInfo const& info)
	: m_Info(info)
{

}

PBE::Engine::~Engine()
{

}

bool PBE::Engine::IsRunning()
{
	return m_Running && !m_pWindow->GetWindowShouldClose();
}

void PBE::Engine::Quit()
{
	m_Running = false;
	m_pWindow->SetWindowShouldClose(true);
}

void PBE::Engine::Startup()
{
	PBE::Clock::InitializeSystemClock();
	InitImGui();

	if (m_Info.m_EnableWindow)
	{
		m_pWindow = new PBE::Window(m_Info.m_WindowInfo);
		m_pWindow->Startup();
		m_Info.m_RendererInfo.m_pWindow = m_pWindow;
		m_pRenderer = new PBE::Renderer(m_Info.m_RendererInfo);
		if (m_Info.m_EnableInputSystem)
		{
			m_Info.m_InputSystemInfo.m_pWindow = m_pWindow;
			m_pInputSystem = new PBE::InputSystem(m_Info.m_InputSystemInfo);
		}
		if (m_Info.m_EnableFontSystem)
		{
			m_Info.m_FontSystemInfo.m_pRenderer = m_pRenderer;
			m_pFontSystem = new PBE::BitmapFontSystem(m_Info.m_FontSystemInfo);
		}
	}
	else
	{
		if (m_Info.m_EnableWindow)
		{
			std::cout << "Error: Cannot create a window without a renderer" << std::endl;
		}
		if (m_Info.m_EnableInputSystem)
		{
			std::cout << "Error: Cannot create an input system without a renderer" << std::endl;
		}
		if (m_Info.m_EnableFontSystem)
		{
			std::cout << "Error: Cannot create a font system without a renderer" << std::endl;
		}
	}
	// No dependency systems
	if (m_Info.m_EnableAudio)
	{
		m_pAudioSystem = new PBE::AudioSystem(m_Info.m_AudioSystemInfo);
		m_pAudioSystem->Startup();
	}
	if (m_Info.m_EnableLua)
	{
		m_pLuaSystem = new PBE::LuaSystem(m_Info.m_LuaSystemInfo);
	}

	if (m_Info.m_EnableJobSystem)
	{
		m_pJobSystem = new PBE::JobSystem(m_Info.m_numJobThreads);
	}

	CreateDefaultResources();

	PBE::DevConsoleConfig config;
	config.m_pWindow = m_pWindow;

	m_pConsole = new DevConsole(config);
	m_pDevConsoleGUI = new PBE::DevConsoleGUI(m_pConsole, m_pWindow);
	m_pDevConsoleGUI->m_NormalizedAnchors = PBE::AABB2(0.f, 0.f, 1.f, 0.3f);

	PBE::CanvasCreateInfo canvasInfo;
	canvasInfo.m_pRenderer = m_pRenderer;
	canvasInfo.m_pTheme = m_DefaultTheme.get();
	canvasInfo.m_pWindow = m_pWindow;
	m_pDebugCanvas = new PBE::Canvas(canvasInfo);
	m_pDebugCanvas->AddChild(m_pDevConsoleGUI);
	m_pRenderer->m_TransferEvent.AddSubscriber([this](Renderer* renderer, VkCommandBuffer cmd, uint32_t frame)
		{
			m_pDebugCanvas->CmdTransfer(renderer, cmd, frame);
			return false;
		}
	);
	m_pRenderer->m_PostProcessEvent.AddSubscriber([this](Renderer* renderer, VkCommandBuffer cmd, uint32_t frame)
		{
			m_pDebugCanvas->CmdPostProcessDraw(cmd, m_pRenderer->GetUIPipelineLayout(), frame);
			return false;
		}
	);

	m_pWindow->m_OnResize.AddSubscriber([this](int width, int height)
		{
			m_pDebugCanvas->OnResize(width, height);
			return false;
		}
	);
	m_pDebugCanvas->OnResize(m_pWindow->GetWidth(), m_pWindow->GetHeight());
	m_pDebugCanvas->UpdateTransform();
}

void PBE::Engine::Shutdown()
{
	m_pRenderer->WaitIdle();
	ImGui_ImplGlfw_Shutdown();

	delete m_pLuaSystem;
	delete m_pDebugCanvas;
	delete m_pDevConsoleGUI;

	m_SpriteAtlas.reset();
	m_DefaultUIMaterial.reset();
	m_DefaultWhiteMaterial.reset();
	m_DefaultFont.reset();
	m_DefaultTheme.reset();

	m_TextureAtlases.clear();
	m_Fonts.clear();
	m_SkeletalModels.clear();
	m_Sounds.clear();
	m_Themes.clear();

	delete m_pFontSystem;
	delete m_pRenderer;
	delete m_pInputSystem;
	delete m_pWindow;
	delete m_pAudioSystem;

	PBE::Clock::Shutdown();
	delete m_pJobSystem;

	ImGui::DestroyContext();
}

void PBE::Engine::CreateDefaultResources()
{
	
	if (m_Info.m_EnableFontSystem)
	{
		BitmapFontCreateInfo loadInfo
		{
			.m_Filename = "Assets/Fonts/Nunito/static/Nunito-SemiBold.ttf",
			.m_Height = 32,
			.m_Glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+[]{}|;:,.<>/?`~'\"\\ 	",
			.m_DropShadow = true,
			.m_ShadowOffset = Vec2(3.f, -3.f),
			.m_ShadowColor = Rgba8(0,0,0, 80)
		};

		m_DefaultFont = LoadOrGetFont("Default",loadInfo);
	}
	
	m_SpriteAtlas = LoadTextureAtlasFromJSON("SpritePage0", "Assets/SpriteAtlas.json", SamplerType::NEAREST_CLAMP);

	if (m_Info.m_EnableWindow)
	{
		if (auto atlas = m_SpriteAtlas.lock())
		{
			MaterialCreateInfo uiMatInfo
			{
				.m_Device = m_pRenderer->m_Device,
				.m_Allocator = m_pRenderer->m_Allocator,
				.m_DiffuseTexture = atlas->GetTexture(),
				.m_SpecularGlossEmitTexture = m_pRenderer->GetWhiteTexture(),
				.m_NormalTexture = m_pRenderer->GetWhiteTexture(),
				.m_ParallaxTexture = m_pRenderer->GetWhiteTexture()
			};

			m_DefaultUIMaterial = std::shared_ptr<Material>(m_pRenderer->CreateMaterial(uiMatInfo));
		}

		MaterialCreateInfo whiteMatInfo
		{
			.m_Device = m_pRenderer->m_Device,
			.m_Allocator = m_pRenderer->m_Allocator,
			.m_DiffuseTexture = m_pRenderer->GetWhiteTexture(),
			.m_SpecularGlossEmitTexture = m_pRenderer->GetWhiteTexture(),
			.m_NormalTexture = m_pRenderer->GetWhiteTexture(),
			.m_ParallaxTexture = m_pRenderer->GetWhiteTexture()
		};

		m_DefaultWhiteMaterial = std::shared_ptr<Material>(m_pRenderer->CreateMaterial(whiteMatInfo));

		NineSliceCreateInfo nineSliceInfo
		{
			.m_CellWidth = 12,
			.m_Mode = NineSliceMode::STRETCH
		};

		nineSliceInfo.AutoSliceTexcoords(m_SpriteAtlas.lock()->GetTextureRegion("FrameDark.png").m_UV);
		NineSlice* nineSlice = new NineSlice(nineSliceInfo);

		m_DefaultTheme = std::make_shared<Theme>();
		m_DefaultTheme->m_Font = m_DefaultFont.lock().get();
		m_DefaultTheme->m_DefaultWhiteMaterial = m_DefaultWhiteMaterial.get();
		m_DefaultTheme->m_Material = m_DefaultUIMaterial.get();
		m_DefaultTheme->m_Backer = nineSlice;
		m_DefaultTheme->m_TextBacker = nineSlice;

	}
}

void PBE::Engine::BeginFrame()
{
	PBE::Clock::TickSystemClock();
	if (m_pRenderer)
	{
		m_pRenderer->BeginFrame();
	}
	if (m_pWindow)
	{
		m_pWindow->PollEvents();
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
		m_pWindow->BeginFrame();
		m_pDebugCanvas->UpdateCursorPos(m_pWindow->GetCursorPos());
		m_pDebugCanvas->Update();
	}
	if (m_pInputSystem)
	{
		m_pInputSystem->BeginFrame();
	}
}

void PBE::Engine::EndFrame()
{
	if (m_pInputSystem)
	{
		m_pInputSystem->EndFrame();
	}
	if (m_pAudioSystem)
	{
		m_pAudioSystem->EndFrame();
	}
	if (m_pWindow)
	{
		m_pWindow->EndFrame();
	}

	if (m_pRenderer)
	{
		m_pRenderer->SubmitCmdDraw();
	}
}

std::weak_ptr<PBE::TextureAtlas> PBE::Engine::LoadTextureAtlasFromJSON(std::string_view uniqueName, std::filesystem::path path, SamplerType type)
{
	std::shared_ptr<PBE::TextureAtlas> atlas = std::make_shared<PBE::TextureAtlas>(PBE::TextureAtlas::LoadOrGetTextureAtlas(m_pRenderer,path,type));

	m_TextureAtlases[std::string(uniqueName)] = atlas;

	return atlas;
}

std::weak_ptr<PBE::TextureAtlas> PBE::Engine::LoadOrGetTextureAtlas(std::string_view uniqueName, TextureAtlasCreateInfo const& info)
{
	auto itr = m_TextureAtlases.find(std::string(uniqueName));
	if (itr != m_TextureAtlases.end())
	{
		return itr->second;
	}

	m_TextureAtlases[std::string(uniqueName)] = std::make_shared<PBE::TextureAtlas>(info);
	return m_TextureAtlases[std::string(uniqueName)];
}

std::weak_ptr<PBE::TextureAtlas> PBE::Engine::GetTextureAtlas(std::string_view uniqueName)
{
	auto itr = m_TextureAtlases.find(uniqueName.data());
	if (itr != m_TextureAtlases.end())
	{
		return itr->second;
	}

	return std::weak_ptr<PBE::TextureAtlas>();
}

std::weak_ptr<PBE::Material> PBE::Engine::LoadOrGetMaterial(std::string_view uniqueName, MaterialCreateInfo const& info)
{
	auto itr = m_Materials.find(std::string(uniqueName));
	if (itr != m_Materials.end())
	{
		return itr->second;
	}

	PBE::Material* material = m_pRenderer->CreateMaterial(info);
	std::shared_ptr<PBE::Material> materialPtr(material);
	m_Materials[std::string(uniqueName)] = materialPtr;
	return materialPtr;
}

std::weak_ptr<PBE::Material> PBE::Engine::GetMaterial(std::string_view uniqueName)
{
	auto itr = m_Materials.find(std::string(uniqueName));
	if (itr != m_Materials.end())
	{
		return itr->second;
	}

	return std::weak_ptr<Material>();
}

std::weak_ptr<PBE::BitmapFont> PBE::Engine::LoadOrGetFont(std::string_view uniqueName, BitmapFontCreateInfo const& info)
{
	auto itr = m_Fonts.find(std::string(uniqueName));
	if (itr != m_Fonts.end())
	{
		return itr->second;
	}

	BitmapFont* font = m_pFontSystem->CreateBitmapFont(info);
	std::shared_ptr<PBE::BitmapFont> fontPtr(font);
	m_Fonts[std::string(uniqueName)] = fontPtr;

	return fontPtr;
}

std::weak_ptr<PBE::BitmapFont> PBE::Engine::GetFont(std::string_view uniqueName)
{
	auto itr = m_Fonts.find(uniqueName.data());
	if (itr != m_Fonts.end())
	{
		return itr->second;
	}

	return std::weak_ptr<PBE::BitmapFont>();
}

std::weak_ptr<PBE::Sound> PBE::Engine::LoadOrGetSound(std::string_view uniqueName, std::filesystem::path path, AudioMode mode)
{
	auto itr = m_Sounds.find(std::string(uniqueName));
	if (itr != m_Sounds.end())
	{
		return itr->second;
	}

	PBE::Sound* pSound;
	m_pAudioSystem->CreateSound(path, mode, &pSound);
	if(pSound)
	{
		std::shared_ptr<PBE::Sound> sound( pSound );
		m_Sounds[std::string(uniqueName)] = sound;

		return sound;
	}

	return std::weak_ptr<PBE::Sound>();
}

std::weak_ptr<PBE::Sound> PBE::Engine::GetSound(std::string_view uniqueName)
{
	auto itr = m_Sounds.find(uniqueName.data());
	if (itr != m_Sounds.end())
	{
		return itr->second;
	}

	return std::weak_ptr<PBE::Sound>();
}

void PBE::Engine::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("Assets/Fonts/Nunito/static/Nunito-SemiBold.ttf", 18);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Dark, painterly-inspired old Windows color scheme with balanced buttons & tabs
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.10f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.16f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.09f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.26f, 0.26f, 0.32f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.30f, 0.36f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.60f, 0.40f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.60f, 0.70f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.65f, 0.75f, 1.00f);

	// Slightly dark but not too dark buttons
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.30f, 1.00f);

	// Softer muted blue-gray tabs
	colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);

	colors[ImGuiCol_Header] = ImVec4(0.14f, 0.14f, 0.18f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.26f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.30f, 0.36f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.55f, 0.65f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.60f, 0.70f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.18f, 0.40f, 0.50f, 0.35f);

	// Style adjustments for a balanced, classic feel
	style.WindowRounding = 6.0f;
	style.FrameRounding = 4.0f;
	style.ScrollbarRounding = 6.0f;
	style.GrabRounding = 4.0f;
	style.ChildRounding = 4.0f;

	style.WindowTitleAlign = ImVec2(0.00f, 0.50f);
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.FramePadding = ImVec2(6.0f, 3.0f);
	style.ItemSpacing = ImVec2(6.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.IndentSpacing = 18.0f;

	style.ScrollbarSize = 14.0f;
	style.GrabMinSize = 8.0f;

	style.AntiAliasedLines = true;
	style.AntiAliasedFill = true;
}

