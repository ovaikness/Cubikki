#pragma once
#include "Cubikki/Actors/Actor.hpp"
#include "Engine/Input/InputActionContext.hpp"

class APawn;
class AController : public Actor
{
public:
	ActorUID m_ControlledPawnUID;
	PBE::InputActionContext m_ActionContext;
	PBE::SubscriberHandle<uint32_t, PBE::NamedProperties&> m_PossessHandle;
	PBE::SubscriberHandle<uint32_t, PBE::NamedProperties&> m_UnpossessHandle;
public:
	AController(PBE::NamedProperties const& properties);

	virtual void Possess(ActorUID pawnUID);
	virtual void Unpossess();

	APawn* GetControlledPawn() const;

	virtual void ImGui() override;
};