#include "Cubikki/Actors/ActorClasses/AController.hpp"
#include "Cubikki/Actors/ActorClasses/APawn.hpp"
#include "Cubikki/Actors/ActorSystem.hpp"

#include "Cubikki/Game/Game.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"

#include "imgui.h"

AController::AController(PBE::NamedProperties const& properties)
	: Actor(properties)
{
	if (g_BlockWorld->m_NetMode != BlockWorldNetMode::SERVER)
	{
		//#TODO Fix sign possible issue with casting from int to uint32_t
		m_PossessHandle = SubscribeToRemoteEvent("Possess", [this](uint32_t id, PBE::NamedProperties& props) -> bool
			{
				if (!props.HasProperty("controller"))
				{
					return false;
				}
				if (std::get<int>(props["controller"]) != (int)m_NetUID)
				{
					return false;
				}
				Actor* pawn = m_System->GetActorByNetUID((uint32_t)std::get<int>(props["pawn"]));
				Possess(pawn->GetUID());
				return false;
			}
		);
		m_UnpossessHandle = SubscribeToRemoteEvent("Unpossess", [this](uint32_t id, PBE::NamedProperties& props) -> bool
			{
				if (!props.HasProperty("controller"))
				{
					return false;
				}
				if (std::get<int>(props["controller"]) != (int)m_NetUID)
				{
					return false;
				}
				Unpossess();
				return false;
			}
		);
	}
}

void AController::Possess(ActorUID pawnUID)
{
	m_ControlledPawnUID = pawnUID;
	APawn* pawn = GetControlledPawn();
	if (pawn)
	{
		pawn->m_ControllerUID = m_UID;
		pawn->OnPossessed(m_UID);

		if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
		{
			PBE::NamedProperties props;
			props["controller"] = (int)m_NetUID;
			props["pawn"] = (int)pawn->m_NetUID;
			SendRemoteEventToAllClients("Possess", props);
		}
	}
}

void AController::Unpossess()
{
	APawn* pawn = GetControlledPawn();
	if (pawn)
	{
		pawn->m_ControllerUID = INVALID_UID;
		pawn->OnUnpossessed();

		if (g_BlockWorld->m_NetMode == BlockWorldNetMode::SERVER)
		{
			PBE::NamedProperties props;
			props["controller"] = (int)m_NetUID;
			props["pawn"] = (int)pawn->m_NetUID;
			SendRemoteEventToAllClients("Unpossess", props);
		}
	}
	m_ControlledPawnUID = INVALID_UID;
}

APawn* AController::GetControlledPawn() const
{
	return dynamic_cast<APawn*>(m_System->GetActor(m_ControlledPawnUID));
}

void AController::ImGui()
{
	using namespace PBE;
	std::vector<Actor*> const& actors = m_System->m_Actors;
	std::vector<bool> selectedActors;
	selectedActors.resize(actors.size(), true);

	ImGui::Text("Possessed Actor");
	for (size_t i = 0; i < actors.size(); ++i)
	{
		Actor* actor = actors[i];
		APawn* pawn = dynamic_cast<APawn*>(actor);
		if (pawn)
		{
			std::string label = PBE::Stringf("%s_%i", pawn->m_name.c_str(), pawn->m_UID.m_UID);
			if (actor->m_UID == m_ControlledPawnUID)
			{
				ImGui::Text(label.c_str());
			}
			else
			{
				if (ImGui::Selectable(label.c_str(), selectedActors[i]))
				{
					Possess(pawn->m_UID);
				}
			}
		}
	}
}


