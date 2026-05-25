#include "Cubikki/Blocks/BlockIterator.hpp"

#include "Cubikki/Blocks/BlockCommon.hpp"
#include "Cubikki/Blocks/Chunk.hpp"

BlockIterator::BlockIterator()
	: m_pChunk(nullptr)
	, m_Index(0)
{
}

BlockIterator::BlockIterator(Chunk* chunk, int index)
	: m_pChunk(chunk)
	, m_Index(index)
{
}

BlockIterator::~BlockIterator()
{

}

PBE::IntVec3 BlockIterator::GetLocalBlockPos() const
{
	if (m_pChunk)
	{
		return PBE::IntVec3(
			GetLocalBlockXFromBlockIndex(m_Index),
			GetLocalBlockYFromBlockIndex(m_Index),
			GetLocalBlockZFromBlockIndex(m_Index)
		);
	}
	return PBE::IntVec3();
}

PBE::IntVec3 BlockIterator::GetIntWorldPosition() const
{
	if (m_pChunk)
	{
		PBE::IntVec3 blockPos = GetLocalBlockPos();
		return blockPos + 
			PBE::IntVec3(
				m_pChunk->m_ChunkCoords.x * CHUNK_X_SIZE,
				m_pChunk->m_ChunkCoords.y * CHUNK_Y_SIZE,
				m_pChunk->m_ChunkCoords.z * CHUNK_Z_SIZE
			);
	}
	return PBE::IntVec3();
}

PBE::Vec3 BlockIterator::GetWorldPosition() const
{
	if (m_pChunk)
	{
		PBE::IntVec3 blockPos = GetLocalBlockPos();
		return PBE::Vec3(blockPos) + PBE::Vec3(
			(float)m_pChunk->m_ChunkCoords.x * CHUNK_X_SIZE,
			(float)m_pChunk->m_ChunkCoords.y * CHUNK_Y_SIZE,
			(float)m_pChunk->m_ChunkCoords.z * CHUNK_Z_SIZE
		);
	}
	return PBE::Vec3();
}

PBE::AABB3 BlockIterator::GetBlockAABB() const
{
	if (m_pChunk)
	{
		PBE::Vec3 blockPos = GetWorldPosition();
		return PBE::AABB3(
			blockPos,
			blockPos + PBE::Vec3(1.f, 1.f, 1.f)
		);
	}
	return PBE::AABB3();
}

BlockIterator BlockIterator::GetNeighbor(BlockFace face) const
{
	switch (face)
	{
		case BlockFace::TOP: return GetUp();
		case BlockFace::BOTTOM: return GetDown();
		case BlockFace::FRONT: return GetForward();
		case BlockFace::BACK: return GetBack();
		case BlockFace::LEFT: return GetLeft();
		case BlockFace::RIGHT: return GetRight();
	}
	return *this;
}

BlockIterator BlockIterator::GetUp() const
{
	BlockIterator result;
	int z = GetLocalBlockZFromBlockIndex(m_Index);

	if (z == CHUNK_Z_SIZE - 1)
	{
		if (m_pChunk)
		{
			result.m_pChunk = m_pChunk->m_ChunkUp;
		}
		result.m_Index = SetLocalBlockZInBlockIndex(m_Index, 0);
	}
	else
	{
		result.m_pChunk = m_pChunk;
		result.m_Index = SetLocalBlockZInBlockIndex(m_Index, z + 1);
	}

	return result;

}

BlockIterator BlockIterator::GetDown() const
{
	BlockIterator result;
	int z = GetLocalBlockZFromBlockIndex(m_Index);

	if (z == 0)
	{
		if (m_pChunk)
		{
			result.m_pChunk = m_pChunk->m_ChunkDown;
		}
		result.m_Index = SetLocalBlockZInBlockIndex(m_Index, CHUNK_Z_SIZE - 1);
	}
	else
	{
		result.m_pChunk = m_pChunk;
		result.m_Index = SetLocalBlockZInBlockIndex(m_Index, z - 1);
	}

	return result;
}

BlockIterator BlockIterator::GetForward() const
{
	BlockIterator result;
	int x = GetLocalBlockXFromBlockIndex(m_Index);
	if (x == CHUNK_X_SIZE - 1)
	{
		if (m_pChunk)
		{
			result.m_pChunk = m_pChunk->m_ChunkForward;
		}
		result.m_Index = SetLocalBlockXInBlockIndex(m_Index, 0);
	}
	else
	{
		result.m_pChunk = m_pChunk;
		result.m_Index = SetLocalBlockXInBlockIndex(m_Index, x + 1);
	}

	return result;
}

BlockIterator BlockIterator::GetBack() const
{
	BlockIterator result;
	int x = GetLocalBlockXFromBlockIndex(m_Index);
	if (x == 0)
	{
		if (m_pChunk)
		{
			result.m_pChunk = m_pChunk->m_ChunkBack;
		}
		result.m_Index = SetLocalBlockXInBlockIndex(m_Index, CHUNK_X_SIZE - 1);
	}
	else
	{
		result.m_pChunk = m_pChunk;
		result.m_Index = SetLocalBlockXInBlockIndex(m_Index, x - 1);
	}

	return result;
}

BlockIterator BlockIterator::GetLeft() const
{
	BlockIterator result;
	int y = GetLocalBlockYFromBlockIndex(m_Index);
	if (y == CHUNK_Y_SIZE - 1)
	{
		if (m_pChunk)
		{
			result.m_pChunk = m_pChunk->m_ChunkLeft;
		}
		result.m_Index = SetLocalBlockYInBlockIndex(m_Index, 0);
	}
	else
	{
		result.m_pChunk = m_pChunk;
		result.m_Index = SetLocalBlockYInBlockIndex(m_Index, y + 1);
	}

	return result;
}

BlockIterator BlockIterator::GetRight() const
{
	BlockIterator result;
	int y = GetLocalBlockYFromBlockIndex(m_Index);
	if (y == 0)
	{
		if (m_pChunk)
		{
			result.m_pChunk = m_pChunk->m_ChunkRight;
		}
		result.m_Index = SetLocalBlockYInBlockIndex(m_Index, CHUNK_Y_SIZE - 1);
	}
	else
	{
		result.m_pChunk = m_pChunk;
		result.m_Index = SetLocalBlockYInBlockIndex(m_Index, y - 1);
	}

	return result;
}

BlockIterator& BlockIterator::operator++()
{
	m_Index++;
	return *this;
}

BlockTypeID BlockIterator::GetBlockTypeID() const
{
	if (m_pChunk)
	{
		Block const& block = m_pChunk->m_BlockStorage.GetBlock(m_Index);
		return block.m_BlockID;
	}
	return 0;
}

BlockDef* BlockIterator::GetBlockDef() const
{
	if (m_pChunk)
	{
		Block const& block = m_pChunk->m_BlockStorage.GetBlock(m_Index);
		return &g_BlockDefs[block.m_BlockID];
	}
	return nullptr;
}

BlockLight* BlockIterator::GetLight() const
{
	return m_pChunk->m_Lights + m_Index;
}

bool BlockIterator::operator!=(const BlockIterator& other) const
{
	return m_Index != other.m_Index || m_pChunk != other.m_pChunk;
}

bool BlockIterator::IsValid() const
{
	return m_pChunk && m_Index >= 0 && m_Index < CHUNK_BLOCK_COUNT;
}

bool BlockIterator::IsEnd() const
{
	return m_Index >= CHUNK_BLOCK_COUNT;
}

bool BlockIterator::operator==(const BlockIterator& other) const
{
	return m_Index == other.m_Index && m_pChunk == other.m_pChunk;
}
