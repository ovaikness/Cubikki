#pragma once

#include "Cubikki/Blocks/BlockIterator.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"

#include "Cubikki/Actors/Component/Component.hpp"

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Rotor3D.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"

#include <cinttypes>

class BlockPhysicsWorld;

struct BlockPhysicsBodyCreateInfo
{
	PBE::OBB3 m_OBB;
	bool  m_HasGravity = true;
	float m_Mass = 1.f;
	float m_Restitution = 0.5f;
	float m_Friction = 0.5f;
	float m_Drag = 0.5f;
	float m_AngularDrag = 0.5f;
	int m_CollisionGroup = 1;
};

class BlockPhysicsBodyComponent : public Component
{
public:
	PBE::OBB3 m_OBB;

	int m_CollisionGroup = 1;
	
	bool m_HasGravity = true;
	float m_Mass;
	float m_InvMass;

	float m_MomentOfInertia;
	float m_InvMomentOfInertia;

	float m_Restitution;
	float m_Friction;
	float m_Drag;
	float m_AngularDrag;
	float m_StepHeight = 1.1f;

	PBE::Vec3 m_Force;
	PBE::Vec3 m_Velocity;
	PBE::Vec3 m_Position;

	PBE::Rotor3D m_RotationalInertia;

	PBE::EulerAngles m_Rotation;
public:
	BlockPhysicsBodyComponent() = default;
	BlockPhysicsBodyComponent(std::string_view name, BlockPhysicsBodyCreateInfo const& info, BlockPhysicsWorld* pWorld);
	void SetOBB(PBE::OBB3 const& obb);
	PBE::OBB3 const& GetOBB() const;

	PBE::Vec3 GetPosition() const;
	void SetPosition(PBE::Vec3 const& position);

	bool IsOnGround() const;

	void FixedUpdate(float deltaSeconds);
	void AddForce(PBE::Vec3 const& force);
	void AddImpulse(PBE::Vec3 const& impulse);
	void AddTorque(PBE::Rotor3D const& torque);

	virtual void OnAttach(Actor* actor) override;
	virtual void OnDetach(Actor* actor) override;
	virtual void ImGui() override;
protected:
	friend class BlockPhysicsWorld;
	void PushOutOfBlock(BlockIterator const& blockItr, PBE::Vec3 const& velocity);
	void PushOutOfOtherCollider(BlockPhysicsBodyComponent& other);
private:
	BlockPhysicsWorld* m_pWorld = nullptr;
};