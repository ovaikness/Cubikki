#pragma once

#include "Cubikki/Blocks/BlockCommon.hpp"

#include "Engine/Math/IntVec3.hpp"

#include <fstream>
#include <filesystem>
#include <map>

class BlockWorld;

struct ChunkRegionHeader
{
	char m_Header[8];
};

struct ChunkCompressedData
{
	PBE::IntVec3 m_ChunkPos;
	std::vector<uint8_t> m_CompressedBlockIndices;
	std::vector<uint8_t> m_CompressedExtendedBlockIndices;
	std::vector<uint8_t> m_CompressedBlockData;
};

class ChunkRegion
{
public:
	std::filesystem::path m_Path;
	BlockWorld* m_BlockWorld;
	ChunkRegionHeader m_Header;
	ChunkCompressedData* m_ChunkData[CHUNK_REGION_CHUNK_COUNT] { nullptr };
public:
	ChunkRegion(BlockWorld* blockWorld, std::filesystem::path path);
	~ChunkRegion();

	ChunkCompressedData* GetChunkCompressedDataPointer(PBE::IntVec3 chunkPos);
	void SetChunkCompressedDataPointer(PBE::IntVec3 chunkPos, ChunkCompressedData* data);

	void SaveData();
};