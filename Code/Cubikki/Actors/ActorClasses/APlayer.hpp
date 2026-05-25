#pragma once

#include "Cubikki/Actors/Actor.hpp"
#include "Cubikki/Actors/ActorClasses/APawn.hpp"
#include "Cubikki/Game/Inventory/Inventory.hpp"

#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Graphics/Camera.hpp"

class APlayer : public APawn
{
public:
	ActorUID m_ControllerUID;
	ComponentReference m_Model;
	ComponentReference m_Collider;
	ComponentReference m_ModelOffsetTransform;

	std::string m_Animation;

	int m_blockIndex = 1;
	Inventory m_Inventory;
	ItemSlot  m_CursorSlot{ nullptr, 0 };
public:
	APlayer(PBE::NamedProperties const& properties);
	~APlayer();

	virtual void OnPossessed(ActorUID actor) override;
	virtual void OnUnpossessed() override;

	virtual void BeginFrame() override;
	virtual void Update() override;
	virtual void EndFrame() override;

	virtual void ImGui() override;

	void Translate(PBE::Vec3 const& translation);
	void Rotate(PBE::EulerAngles const& rotation);
};