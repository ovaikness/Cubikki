#include "Cubikki/Blocks/Physics/BlockPhysicsBody.hpp"
#include "Cubikki/Blocks/Physics/BlockPhysicsWorld.hpp"

#include <iostream>
#include "imgui.h"

BlockPhysicsBodyComponent::BlockPhysicsBodyComponent(std::string_view name, BlockPhysicsBodyCreateInfo const& info, BlockPhysicsWorld* pWorld)
	: Component(name)
	, m_pWorld(pWorld)
{
	m_OBB = info.m_OBB;
	m_Mass = info.m_Mass;
	m_InvMass = 1.f / m_Mass;
	m_Restitution = info.m_Restitution;
	m_Friction = info.m_Friction;
	m_Drag = info.m_Drag;
	m_AngularDrag = info.m_AngularDrag;
	m_HasGravity = info.m_HasGravity;
	m_Position = m_OBB.GetCenter();
	m_CollisionGroup = info.m_CollisionGroup;
}

void BlockPhysicsBodyComponent::SetOBB(PBE::OBB3 const& obb)
{
	m_OBB = obb;
}

PBE::OBB3 const& BlockPhysicsBodyComponent::GetOBB() const
{
	return m_OBB;
}

PBE::Vec3 BlockPhysicsBodyComponent::GetPosition() const
{
	return m_Position;
}

void BlockPhysicsBodyComponent::SetPosition(PBE::Vec3 const& position)
{
	m_Position = position;
	m_OBB.SetCenter(position);
}

bool BlockPhysicsBodyComponent::IsOnGround() const
{
	auto corners = m_OBB.GetCorners();
	for (PBE::Vec3 const& corner : corners)
	{
		PBE::IntVec3 flatCorner = PBE::IntVec3(PBE::Ceil(corner.x), PBE::Ceil(corner.y), PBE::Ceil(corner.z - 1));
		BlockIterator cornerItr = m_pWorld->m_Info.m_pWorld->GetBlockIterator(flatCorner);
		if (cornerItr.IsValid())
		{
			BlockDef* blockDef = cornerItr.GetBlockDef();
			if (blockDef->m_IsSolid)
			{
				return true;
			}
		}
	}

	return false;
}

void BlockPhysicsBodyComponent::FixedUpdate(float deltaSeconds)
{
	PBE::Vec3 prevPosition = m_Position;
	auto corners = m_OBB.GetCorners();

	PBE::AABB3 boundingBox;
	boundingBox.m_Mins = m_OBB.GetCenter();
	boundingBox.m_Maxs = m_OBB.GetCenter();
	for (PBE::Vec3 const& corner : corners)
	{
		boundingBox.Enclose(corner);
	}

	PBE::IntVec3 flatCenter = PBE::IntVec3(PBE::Ceil(boundingBox.GetCenter().x), PBE::Ceil(boundingBox.GetCenter().y), PBE::Ceil(boundingBox.GetCenter().z));
	BlockIterator centerItr = m_pWorld->m_Info.m_pWorld->GetBlockIterator(flatCenter);

	if (centerItr.m_pChunk)
	{
		if (centerItr.m_pChunk->m_State != ChunkState::COMPLETE)
		{
			return;
		}
	}
	else
	{
		return;
	}

	PBE::Vec3 translation(0.f);

	PBE::Vec3 gravity;
	if (m_HasGravity)
	{
		gravity = m_pWorld->m_Info.m_Gravity;
	}
	else
	{
		gravity = PBE::Vec3::ZERO;
	}

	m_Velocity += gravity * deltaSeconds * 0.5f;
	m_Velocity += m_Force * m_InvMass * deltaSeconds * 0.5f;
	translation += m_Velocity * deltaSeconds;
	m_Velocity += m_Force * m_InvMass * deltaSeconds * 0.5f;
	m_Velocity += gravity * deltaSeconds * 0.5f;

	m_Position += translation;

	m_Force = PBE::Vec3::ZERO;

	m_OBB.SetCenter(m_Position);

	auto centersPost = m_OBB.GetCorners();
	auto bboxPost = PBE::AABB3();
	bboxPost.m_Mins = m_OBB.GetCenter();
	bboxPost.m_Maxs = m_OBB.GetCenter();
	for (PBE::Vec3 const& corner : centersPost)
	{
		bboxPost.Enclose(corner);
	}

	PBE::IntVec3 flatMins = PBE::IntVec3((int)bboxPost.m_Mins.x, (int)bboxPost.m_Mins.y, (int)bboxPost.m_Mins.z);
	PBE::IntVec3 flatMaxs = PBE::IntVec3((int)std::ceil(bboxPost.m_Maxs.x), (int)std::ceil(bboxPost.m_Maxs.y), (int)std::ceil(bboxPost.m_Maxs.z));

	BlockIterator blockItr = m_pWorld->m_Info.m_pWorld->GetBlockIterator(flatMins);

	for (int xx = flatMins.x - 1; xx <= flatMaxs.x + 1; ++xx)
	{
		for (int yy = flatMins.y - 1; yy <= flatMaxs.y + 1; ++yy)
		{
			for (int zz = flatMins.z - 1; zz <= flatMaxs.z + 1; ++zz)
			{
				blockItr = m_pWorld->m_Info.m_pWorld->GetBlockIterator(PBE::IntVec3(xx, yy, zz));
				if (blockItr.IsValid())
				{
					if (m_CollisionGroup > 0)
					{
						PushOutOfBlock(blockItr, translation);
					}
				}
				else
				{
					m_Position = prevPosition;
					m_OBB.SetCenter(m_Position);
					m_Velocity = PBE::Vec3::ZERO;
					return;
				}
				blockItr = blockItr.GetUp();
			}
			blockItr = blockItr.GetLeft();
		}
		blockItr = blockItr.GetForward();
	}

	m_Position = m_OBB.GetCenter();
	float z = m_Velocity.z;
	m_Velocity *= m_Drag;
	m_Velocity.z = z;
}

void BlockPhysicsBodyComponent::AddForce(PBE::Vec3 const& force)
{
	m_Force += force;
}

void BlockPhysicsBodyComponent::AddImpulse(PBE::Vec3 const& impulse)
{
	m_Velocity += impulse * m_InvMass;
}

void BlockPhysicsBodyComponent::AddTorque(PBE::Rotor3D const& torque)
{
	m_RotationalInertia += torque;
}

void BlockPhysicsBodyComponent::OnAttach(Actor* actor)
{

}

void BlockPhysicsBodyComponent::OnDetach(Actor* actor)
{

}

void BlockPhysicsBodyComponent::ImGui()
{
	ImGui::DragFloat3("Position : ", &m_Position.x);
	ImGui::Text("Velocity: %f, %f, %f", m_Velocity.x, m_Velocity.y, m_Velocity.z);
	ImGui::Text("Mass: %f", m_Mass);
	ImGui::Text("InvMass: %f", m_InvMass);
	ImGui::Text("Drag: %f", m_Drag);
	ImGui::Text("AngularDrag: %f", m_AngularDrag);
	ImGui::Text("Restitution: %f", m_Restitution);
	ImGui::Text("Friction: %f", m_Friction);
	ImGui::Text("StepHeight: %f", m_StepHeight);
	ImGui::Text("OBB: %f, %f, %f", m_OBB.GetCenter().x, m_OBB.GetCenter().y, m_OBB.GetCenter().z);
	ImGui::Text("OBB Extents: %f, %f, %f", m_OBB.GetWidth(), m_OBB.GetHeight(), m_OBB.GetDepth());
	ImGui::Text("OBB Rotation: %f, %f, %f", m_OBB.GetIBasis().x, m_OBB.GetIBasis().y, m_OBB.GetIBasis().z);
	ImGui::Text("OBB Rotation: %f, %f, %f", m_OBB.GetJBasis().x, m_OBB.GetJBasis().y, m_OBB.GetJBasis().z);
	ImGui::Text("OBB Rotation: %f, %f, %f", m_OBB.GetKBasis().x, m_OBB.GetKBasis().y, m_OBB.GetKBasis().z);
}

void BlockPhysicsBodyComponent::PushOutOfBlock(BlockIterator const& blockItr, PBE::Vec3 const& velocity)
{
	if (blockItr.m_pChunk->m_State != ChunkState::COMPLETE)
	{
		return;
	}

	BlockDef* blockDef = blockItr.GetBlockDef();
	if (blockDef->m_IsSolid)
	{
		//Only step up if the block above current block isn't solid. Otherwise character will tunnel through the block above in a chain.
		bool isUpSolid = true;
		BlockIterator upBlockItr = blockItr.GetUp();
		if (upBlockItr.IsValid())
		{
			BlockDef* upBlockDef = upBlockItr.GetBlockDef();
			if (upBlockDef->m_IsSolid)
			{
				isUpSolid = true;
			}
			else
			{
				isUpSolid = false;
			}
		}

		PBE::AABB3 blockAABB = blockItr.GetBlockAABB();
		PBE::CollisionResult3D result = m_OBB.GetCollision(blockAABB, isUpSolid ? 0.f : m_StepHeight);
	
		if (result.m_Hit)
		{
			m_Position += result.m_HitPushDisplacement;
			m_Velocity = m_Velocity - m_Velocity.Dot(result.m_HitSurfaceNormal) * result.m_HitSurfaceNormal;
		}

		m_OBB.SetCenter(m_Position);
	}
}


void BlockPhysicsBodyComponent::PushOutOfOtherCollider(BlockPhysicsBodyComponent& other)
{
	PBE::CollisionResult3D result = m_OBB.GetCollision(other.m_OBB);

	if (result.m_Hit)
	{
		float massRatio = m_Mass / other.m_Mass;
		m_Position += result.m_HitPushDisplacement * massRatio;
		other.m_Position -= result.m_HitPushDisplacement * (1.f - massRatio);
	}

	m_OBB.SetCenter(m_Position);
	other.m_OBB.SetCenter(other.m_Position);
}
