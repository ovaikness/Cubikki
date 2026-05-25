#pragma once

#include "Cubikki/Actors/Actor.hpp"
#include "Cubikki/Actors/ActorClasses/AController.hpp"

#include "Engine/Graphics/Camera.hpp"

class APlayerController : public AController
{
public:
	uint32_t m_ClientID{ SERVER_CLIENT_ID };
public:
	PBE::Camera* m_Camera{ nullptr };

	APlayerController(PBE::NamedProperties const& properties);
	virtual void Update() override;
};