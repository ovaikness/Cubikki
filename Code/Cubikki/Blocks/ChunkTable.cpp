#include "Cubikki/Blocks/ChunkTable.hpp"

Chunk* ChunkTable::GetChunk(PBE::IntVec3 const& chunkCords)
{
	ChunkTableEntry& entry = GetEntry(chunkCords);
	if (entry.m_NumChunks == 0)
	{
		return nullptr;
	}

	int index = GetChunkIndexFromGlobalChunkCoords(chunkCords);
	return entry.m_Chunks[index];
}

void ChunkTable::SetChunk(PBE::IntVec3 const& pos, Chunk* chunk)
{
	ChunkTableEntry& entry = GetEntry(pos);
	int index = GetChunkIndexFromGlobalChunkCoords(pos);
	Chunk*& chunkRef = entry.m_Chunks[index];

	if (chunkRef == nullptr && chunk == nullptr)
	{
		return;
	}
	else if (chunkRef == nullptr && chunk != nullptr)
	{
		m_NumChunks++;
		entry.m_NumChunks++;
	}
	else if (chunkRef != nullptr && chunk == nullptr)
	{
		m_NumChunks--;
		entry.m_NumChunks--;
	}
	entry.m_Chunks[index] = chunk;

	if (entry.m_NumChunks == 0)
	{
		m_ChunkTableEntries.erase(pos);
	}
}

void ChunkTable::UpdateChunksVector()
{
	m_Chunks.clear();
	for (auto& entry : m_ChunkTableEntries)
	{
		for (int i = 0; i < CHUNK_REGION_CHUNK_COUNT; ++i)
		{
			if (entry.second.m_Chunks[i])
			{
				m_Chunks.push_back(entry.second.m_Chunks[i]);
			}
		}
	}
}

ChunkTableEntry& ChunkTable::GetEntry(PBE::IntVec3 const& chunkCords)
{
	PBE::IntVec3 regionCoords = GetRegionCoordsFromGlobalChunkCoords(chunkCords);
	auto itr = m_ChunkTableEntries.find(regionCoords);
	if (itr == m_ChunkTableEntries.end())
	{
		m_ChunkTableEntries[regionCoords] = {};
	}
	return m_ChunkTableEntries[regionCoords];
}
