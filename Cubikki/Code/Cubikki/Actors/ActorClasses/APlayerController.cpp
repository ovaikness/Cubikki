#include "Cubikki/Actors/ActorClasses/APlayerController.hpp"
#include "Cubikki/Actors/ActorClasses/AFlyCam.hpp"

#include "Cubikki/Blocks/Physics/BlockPhysicsBody.hpp"
#include "Cubikki/Actors/ActorSystem.hpp"

#include "Cubikki/Blocks/BlockWorld.hpp"
#include "Cubikki/Game/Game.hpp"

#include <string>

APlayerController::APlayerController(PBE::NamedProperties const& properties)
	: AController(properties)
{
	using namespace PBE;
	m_name = "PlayerController";
	m_IsNetActor = true;
	m_ClientID = (uint32_t)std::get<int>(properties["ClientID"]);
}

void APlayerController::Update()
{
	if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER && m_ClientID != SERVER_CLIENT_ID)
	{
		return;
	}

	if (g_BlockWorld->m_NetMode == BlockWorldNetMode::CLIENT && m_ClientID != g_Client->GetID())
	{
		return;
	}

	using namespace PBE;
	static bool freeCursor = true;

	if (freeCursor)
	{
		g_Engine->m_pWindow->SetCursorMode(PBE::CursorMode::FPS);
	}
	else
	{
		g_Engine->m_pWindow->SetCursorMode(PBE::CursorMode::NORMAL);
	}

	InputSystem* input = g_Engine->m_pInputSystem;

	if (input->WasKeyPressed(KEY_ENTER))
	{
		freeCursor = !freeCursor;
	}

	bool started = false;
	PBE::Vec2 axis;
	PBE::Vec2 lookAxis;
	if (input->IsKeyHeld(KEY_W))
	{
		started = true;
		axis.x = 1.f;
	}
	if (input->IsKeyHeld(KEY_S))
	{
		started = true;
		axis.x = -1.f;
	}
	if (input->IsKeyHeld(KEY_A))
	{
		started = true;
		axis.y = -1.f;
	}
	if (input->IsKeyHeld(KEY_D))
	{
		started = true;
		axis.y = 1.f;
	}
	axis.Normalize();

	if (g_Engine->m_pWindow->GetCursorMode() == PBE::CursorMode::FPS)
	{
		lookAxis = g_Engine->m_pWindow->GetCursorClientDelta() * 0.05f;
	}

	if (axis != Vec2(0.f, 0.f))
	{
		m_ActionContext.ExecuteAction("Move", axis, PBE::INPUT_ACTION_MODE_HELD);
	}
	
	if (lookAxis != Vec2(0.f, 0.f))
	{
		m_ActionContext.ExecuteAction("Look", lookAxis, PBE::INPUT_ACTION_MODE_HELD);
	}

	if (input->WasKeyPressed(PBE::KEY_SPACE))
	{
		m_ActionContext.ExecuteAction("Jump", true, PBE::INPUT_ACTION_MODE_STARTED);
	}

	if (input->IsKeyHeld(PBE::KEY_SPACE))
	{
		m_ActionContext.ExecuteAction("Elevate/Descend", 1.f, PBE::INPUT_ACTION_MODE_HELD);
	}

	if (input->IsKeyHeld(PBE::KEY_LEFT_SHIFT))
	{
		m_ActionContext.ExecuteAction("Elevate/Descend", -1.f, PBE::INPUT_ACTION_MODE_HELD);
	}

	if (input->IsMouseButtonPressed(PBE::MOUSE_BUTTON_1))
	{
		m_ActionContext.ExecuteAction("PrimaryAction", true, PBE::INPUT_ACTION_MODE_STARTED);
	}
	if (input->IsMouseButtonPressed(PBE::MOUSE_BUTTON_2))
	{
		m_ActionContext.ExecuteAction("SecondaryAction", true, PBE::INPUT_ACTION_MODE_STARTED);
	}

	if (input->WasKeyPressed(PBE::KEY_0))
	{
		m_ActionContext.ExecuteAction("SelectBlock", 1.f, INPUT_ACTION_MODE_STARTED);
	}
	if (input->WasKeyPressed(PBE::KEY_1))
	{
		m_ActionContext.ExecuteAction("SelectBlock", 2.f, INPUT_ACTION_MODE_STARTED);
	}
	if (input->WasKeyPressed(PBE::KEY_2))
	{
		m_ActionContext.ExecuteAction("SelectBlock", 3.f, INPUT_ACTION_MODE_STARTED);
	}
	if (input->WasKeyPressed(PBE::KEY_3))
	{
		m_ActionContext.ExecuteAction("SelectBlock", 7.f, INPUT_ACTION_MODE_STARTED);
	}
	if (input->WasKeyPressed(PBE::KEY_4))
	{
		m_ActionContext.ExecuteAction("SelectBlock", 5.f, INPUT_ACTION_MODE_STARTED);
	}
	if (input->WasKeyPressed(PBE::KEY_5))
	{
		m_ActionContext.ExecuteAction("SelectBlock", 6.f, INPUT_ACTION_MODE_STARTED);
	}
	 
	if (APawn* pawn = dynamic_cast<APawn*>(GetControlledPawn()))
	{
		if (TransformComponent* transform = pawn->GetComponentOfType<TransformComponent>("Transform"))
		{
			PBE::Vec3 translation = g_Game->m_Camera->GetLocalPosition() + (transform->GetPosition() - g_Game->m_Camera->GetLocalPosition()) * 0.05f;
			g_Game->m_Camera->SetOrientation(transform->GetRotation());
			g_Game->m_Camera->SetLocalPosition(translation);
			g_Engine->m_pAudioSystem->SetListenerAttributes(0, transform->GetPosition(), PBE::Vec3(0.f), g_Game->m_Camera->GetForward(), g_Game->m_Camera->GetUp());
		}
	}
}