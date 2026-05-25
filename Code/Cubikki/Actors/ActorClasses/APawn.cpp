#include "Cubikki/Actors/ActorClasses/APawn.hpp"
#include "Cubikki/Actors/ActorClasses/AController.hpp"
#include "Cubikki/Actors/ActorClasses/APlayerController.hpp"
#include "Cubikki/Blocks/Physics/BlockPhysicsBody.hpp"
#include "Cubikki/Actors/ActorSystem.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"

#include "Cubikki/Game/Game.hpp"

APawn::APawn(PBE::NamedProperties const& properties)
	: Actor(properties)
{
	PBE::Vec3 pos;
	properties.GetVec3("Position", pos);
	PBE::EulerAngles orientation;
	properties.GetEulerAngles("Orientation", orientation);

	m_Transform    = AddComponent(g_Game->m_TransformSystem->CreateComponent("Transform", pos, orientation));
	m_NetTransform = AddComponent(g_Game->m_TransformSystem->CreateComponent("Net Transform", pos, orientation));

	if (TransformComponent* transform = m_NetTransform.GetAs<TransformComponent>())
	{
		transform->m_Replicates = true;
	}

	SubscribeToRemoteEvent("SyncPawnToClient", [this](uint32_t id, PBE::NamedProperties& props) -> bool
		{
			uint32_t netID = (uint32_t)std::get<int>(props["NetID"]);
			if (netID != m_NetUID)
			{
				return false;
			}
			if (id != g_Client->GetID() || g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
			{
				if (TransformComponent* transform = m_NetTransform.GetAs<TransformComponent>())
				{
					PBE::Vec3 position = std::get<PBE::Vec3>(props["Position"]);
					PBE::EulerAngles rotation = std::get<PBE::EulerAngles>(props["Rotation"]);

					if (BlockPhysicsBodyComponent* collider = GetComponentOfType<BlockPhysicsBodyComponent>("Collider"))
					{
						collider->SetPosition(position);
					}
					transform->SetPosition(position);
					transform->SetRotation(rotation);
				}
			}
			return false;
		}
	);
}

APawn::~APawn()
{
}

void APawn::OnPossessed(ActorUID actor)
{
	m_ControllerUID = actor;
	AController* controller = GetController();
	if (controller)
	{
		controller->m_ControlledPawnUID = m_UID;
	}
}

void APawn::OnUnpossessed()
{
	AController* controller = GetController();
	if (controller)
	{
		controller->m_ControlledPawnUID = INVALID_UID;
	}
	m_ControllerUID = INVALID_UID;
}

void APawn::BeginFrame()
{
	Actor::BeginFrame();
	AController* controller = GetController();
	APlayerController* playerController = dynamic_cast<APlayerController*>(controller);

	TransformComponent* netTransform = m_NetTransform.GetAs<TransformComponent>();
	TransformComponent* localTransform = m_Transform.GetAs<TransformComponent>();

	//Don't synchronize if we are controlling this pawn. The client is authoritative on the controlled pawns position.
	if (playerController)
	{
		if (g_Client->GetID() != playerController->m_ClientID || g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
		{
			if (localTransform && netTransform)
			{
				localTransform->SetPosition(netTransform->GetPosition());
				localTransform->SetRotation(netTransform->GetRotation());
				localTransform->SetScale(netTransform->GetScale());
			}
		}
	}
	else
	{
		if (localTransform && netTransform)
		{
			localTransform->SetPosition(netTransform->GetPosition());
			localTransform->SetRotation(netTransform->GetRotation());
			localTransform->SetScale(netTransform->GetScale());
		}
	}
}

void APawn::EndFrame()
{
	Actor::EndFrame();

	if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
	{
		if (TransformComponent* localTransform = m_Transform.GetAs<TransformComponent>())
		{
			if (TransformComponent* netTransform = m_NetTransform.GetAs<TransformComponent>())
			{
				netTransform->SetPosition(localTransform->GetPosition());
				netTransform->SetRotation(localTransform->GetRotation());
			}
		}
	}
	else
	{
		AController* controller = GetController();
		APlayerController* playerController = dynamic_cast<APlayerController*>(controller);
		if (playerController)
		{
			if (g_Client->GetID() == playerController->m_ClientID)
			{
				TransformComponent* transform = m_Transform.GetAs<TransformComponent>();
				PBE::NamedProperties props;
				props["Position"] = transform->GetPosition();
				props["Rotation"] = transform->GetRotation();
				props["NetID"]	  = (int)m_NetUID;
				SendRemoteEventToServer("SyncPawnToClient", props);
			}
		}
	}
}

AController* APawn::GetController() const
{
	return dynamic_cast<AController*>(m_System->GetActor(m_ControllerUID));
}

PBE::InputActionContext* APawn::GetActionContext() const
{
	AController* controller = GetController();
	if (controller)
	{
		return &controller->m_ActionContext;
	}
}
;
