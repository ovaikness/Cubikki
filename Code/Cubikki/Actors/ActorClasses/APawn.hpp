#pragma once
#include "Cubikki/Actors/Actor.hpp"
#include "Engine/Input/InputActionContext.hpp"
#include <map>
#include <string>

class AController;
class APawn : public Actor
{
public:
	ActorUID m_ControllerUID;
	ComponentReference m_Transform;
	ComponentReference m_NetTransform;
public:
	APawn(PBE::NamedProperties const& properties);
	virtual ~APawn();

	virtual void OnPossessed(ActorUID actor);
	virtual void OnUnpossessed();

	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	AController* GetController() const;
	PBE::InputActionContext* GetActionContext() const;
};