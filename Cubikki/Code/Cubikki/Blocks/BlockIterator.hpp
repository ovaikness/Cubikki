#pragma once

#include "Cubikki/Blocks/BlockCommon.hpp"
#include "Cubikki/Blocks/BlockLight.hpp"

#include "Engine/Math/AABB3.hpp"

#include <iterator>

class Chunk;
class BlockIterator
{
public:
	Chunk* m_pChunk;
	int m_Index;
public:
	BlockIterator();
	BlockIterator(Chunk* chunk, int index);
	~BlockIterator();

	PBE::IntVec3 GetLocalBlockPos() const;
	PBE::IntVec3 GetIntWorldPosition() const;
	PBE::Vec3 GetWorldPosition() const;
	PBE::AABB3 GetBlockAABB() const;

	BlockIterator GetNeighbor(BlockFace face) const;
	BlockIterator GetUp() const;
	BlockIterator GetDown() const;
	BlockIterator GetForward() const;
	BlockIterator GetBack() const;
	BlockIterator GetLeft() const;
	BlockIterator GetRight() const;

	BlockIterator& operator++();

	BlockTypeID GetBlockTypeID() const;
	BlockDef* GetBlockDef() const;
	BlockLight* GetLight() const;

	bool IsValid() const;
	bool IsEnd() const;
	bool operator==(const BlockIterator& other) const;
	bool operator!=(const BlockIterator& other) const;
};