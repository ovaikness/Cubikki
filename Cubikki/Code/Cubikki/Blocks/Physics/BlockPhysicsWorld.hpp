#pragma once

#include "Cubikki/Blocks/Physics/BlockPhysicsBody.hpp"
#include "Cubikki/Actors/Component/ComponentSystem.hpp"

#include "Engine/Math/Vec3.hpp"

#include <vector>

struct BlockPhysicsWorldCreateInfo
{
	PBE::Vec3 m_Gravity = PBE::Vec3(0.f, 0.f, -9.8f);
	BlockWorld* m_pWorld;
};

class BlockPhysicsWorld : public ComponentSystem<BlockPhysicsBodyComponent>
{
public:
	BlockPhysicsWorldCreateInfo m_Info;
public:
	BlockPhysicsWorld(BlockPhysicsWorldCreateInfo const& info);
	~BlockPhysicsWorld();

	void FixedUpdate(float deltaSeconds);
};