#pragma once
#include "Engine/Core/EventDispatcher.hpp"
#include "Engine/Net/NetCommon.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Core/DevConsole.hpp"

#include "Engine/Graphics/Camera.hpp"

class CubikkiClient;
class CubikkiServer;
class Game;

#ifdef CUBIKKI_APP_IMPLEMENTED
#include "Cubikki/Core/App.hpp"
#else
class App;
#endif

extern App* g_App;
extern PBE::Engine* g_Engine;

extern CubikkiClient* g_Client;
extern CubikkiServer* g_Server;

extern PBE::Camera g_Camera;
extern Game* g_Game;


extern std::weak_ptr<PBE::TextureAtlas> g_BlockAtlas;

constexpr uint32_t SERVER_CLIENT_ID = 0xFFFFFFFF;

extern bool g_ShowDebugInfo;
extern std::string g_IpAddress;
extern uint16_t g_Port;