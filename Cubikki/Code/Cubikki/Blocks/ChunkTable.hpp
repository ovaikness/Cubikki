#pragma once

#include "Cubikki/Blocks/BlockCommon.hpp"
#include "Cubikki/Blocks/Chunk.hpp"

#include "Engine/Math/IntVec3.hpp"

#include <unordered_map>

struct ChunkTableEntry
{
	int m_NumChunks{ 0 };
	Chunk* m_Chunks[CHUNK_REGION_CHUNK_COUNT]{ nullptr };
};

class ChunkTable
{
public:
	int m_NumChunks{ 0 };
public:
	Chunk* GetChunk(PBE::IntVec3 const& chunkCords);
	void SetChunk(PBE::IntVec3 const& pos, Chunk* chunk);
	void UpdateChunksVector();

	std::unordered_map<PBE::IntVec3, ChunkTableEntry> m_ChunkTableEntries;
	std::vector<Chunk*> m_Chunks;
private:
	ChunkTableEntry& GetEntry(PBE::IntVec3 const& chunkCords);
};