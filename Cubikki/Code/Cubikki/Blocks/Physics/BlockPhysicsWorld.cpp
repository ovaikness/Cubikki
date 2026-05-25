#include "Cubikki/Blocks/Physics/BlockPhysicsWorld.hpp"

BlockPhysicsWorld::BlockPhysicsWorld(BlockPhysicsWorldCreateInfo const& info)
	: m_Info(info)
{

}

BlockPhysicsWorld::~BlockPhysicsWorld()
{

}

void BlockPhysicsWorld::FixedUpdate(float deltaSeconds)
{
	for (int i = 0; i < m_Components.m_Entries.size(); ++i)
	{
		if (!m_Components.m_Entries[i].m_InUse)
		{
			continue;
		}
		for (int j = i + 1; j < m_Components.m_Entries.size(); ++j)
		{
			if (!m_Components.m_Entries[j].m_InUse)
			{
				continue;
			}
			m_Components.m_Entries[i].m_Component.PushOutOfOtherCollider(m_Components.m_Entries[j].m_Component);
		}
	}

	for (auto& entry : m_Components)
	{
		if (entry.m_InUse)
		{
			entry.m_Component.FixedUpdate(deltaSeconds);
		}
	}
}
