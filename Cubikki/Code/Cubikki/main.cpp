#include "Cubikki/Common.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Graphics/Window.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/OBB3.hpp"

#include "Cubikki/App.hpp"

int main(int argc, char* argv[])
{
	using namespace PBE;
	
	App* app;
	ApplicationCreateInfo appInfo;
	appInfo.IconPath		= "Assets/Textures/Icon.png";
	appInfo.ApplicationName = "Cubikki";
	appInfo.EngineName		= "PBE";

	app = new App(appInfo);
	app->Startup();
	app->Run();

	delete app;
	return 0;
}