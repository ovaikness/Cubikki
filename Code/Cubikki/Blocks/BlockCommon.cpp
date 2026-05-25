#include "Cubikki/Blocks/BlockCommon.hpp"

int GetLocalBlockXFromBlockIndex(int blockIndex)
{
	return (blockIndex >> CHUNK_X_SHIFT) & CHUNK_X_MASK;
}

int GetLocalBlockYFromBlockIndex(int blockIndex)
{
	return (blockIndex >> CHUNK_Y_SHIFT) & CHUNK_Y_MASK;
}

int GetLocalBlockZFromBlockIndex(int blockIndex)
{
	return (blockIndex >> CHUNK_Z_SHIFT) & CHUNK_Z_MASK;
}

int SetLocalBlockXInBlockIndex(int blockIndex, int x)
{
	return (blockIndex & ~(CHUNK_X_MASK << CHUNK_X_SHIFT)) | (x << CHUNK_X_SHIFT);
}

int SetLocalBlockYInBlockIndex(int blockIndex, int y)
{
	return (blockIndex & ~(CHUNK_Y_MASK << CHUNK_Y_SHIFT)) | (y << CHUNK_Y_SHIFT);
}

int SetLocalBlockZInBlockIndex(int blockIndex, int z)
{
	return (blockIndex & ~(CHUNK_Z_MASK << CHUNK_Z_SHIFT)) | (z << CHUNK_Z_SHIFT);
}

PBE::IntVec3 GetLocalBlockCoordsFromGlobalBlockCoords(PBE::IntVec3 blockCoords)
{
	return PBE::IntVec3(
		blockCoords.x & CHUNK_X_MASK,
		blockCoords.y & CHUNK_Y_MASK,
		blockCoords.z & CHUNK_Z_MASK
	);
}

PBE::IntVec3 GetChunkCoordsFromBlockCoords(PBE::IntVec3 blockCoords)
{
	return PBE::IntVec3(
		blockCoords.x >> CHUNK_X_BITS,
		blockCoords.y >> CHUNK_Y_BITS,
		blockCoords.z >> CHUNK_Z_BITS
	);
}

PBE::IntVec3 GetBlockCoordsFromChunkCoords(PBE::IntVec3 chunkCoords)
{
	return PBE::IntVec3(
		chunkCoords.x << CHUNK_X_BITS,
		chunkCoords.y << CHUNK_Y_BITS,
		chunkCoords.z << CHUNK_Z_BITS
	);
}

PBE::IntVec3 GetLocalBlockCoordsFromBlockIndex(int blockIndex)
{
	return PBE::IntVec3(
		GetLocalBlockXFromBlockIndex(blockIndex),
		GetLocalBlockYFromBlockIndex(blockIndex),
		GetLocalBlockZFromBlockIndex(blockIndex)
	);
}

int GetBlockIndexFromLocalBlockCoords(PBE::IntVec3 blockCoords)
{
	return (blockCoords.x << CHUNK_X_SHIFT) | (blockCoords.y << CHUNK_Y_SHIFT) | (blockCoords.z << CHUNK_Z_SHIFT);
}

PBE::IntVec3 GetLocalChunkCoordsFromGlobalChunkCoords(PBE::IntVec3 chunkCoords)
{
	return PBE::IntVec3(
		chunkCoords.x & CHUNK_REGION_X_MASK,
		chunkCoords.y & CHUNK_REGION_Y_MASK,
		chunkCoords.z & CHUNK_REGION_Z_MASK
	);
}

int GetChunkIndexFromGlobalChunkCoords(PBE::IntVec3 chunkCoords)
{
	return 
		((chunkCoords.x & CHUNK_REGION_X_MASK) << CHUNK_REGION_X_SHIFT) | 
		((chunkCoords.y & CHUNK_REGION_Y_MASK) << CHUNK_REGION_Y_SHIFT) | 
		((chunkCoords.z & CHUNK_REGION_Z_MASK) << CHUNK_REGION_Z_SHIFT);
}

int GetChunkIndexFromLocalChunkCoords(PBE::IntVec3 chunkCoords)
{
	return 
		(chunkCoords.x << CHUNK_REGION_X_SHIFT) |
		(chunkCoords.y << CHUNK_REGION_Y_SHIFT) |
		(chunkCoords.z << CHUNK_REGION_Z_SHIFT);
}

PBE::IntVec3 GetRegionCoordsFromGlobalChunkCoords(PBE::IntVec3 chunkCoords)
{
	return PBE::IntVec3(
		chunkCoords.x >> CHUNK_REGION_X_BITS,
		chunkCoords.y >> CHUNK_REGION_Y_BITS,
		chunkCoords.z >> CHUNK_REGION_Z_BITS
	);
}

PBE::IntVec3 GetChunkCoordsFromRegionCoords(PBE::IntVec3 regionCoords)
{
	return PBE::IntVec3(
		regionCoords.x << CHUNK_REGION_X_BITS,
		regionCoords.y << CHUNK_REGION_Y_BITS,
		regionCoords.z << CHUNK_REGION_Z_BITS
	);
}

PBE::Vec3 GetBlockFaceOffset(BlockFace face)
{
	switch (face)
	{
		case BOTTOM: return PBE::Vec3(0, 0, -1);
		case TOP: return PBE::Vec3(0, 0, 1);
		case BACK: return PBE::Vec3(-1, 0, 0);
		case FRONT: return PBE::Vec3(1, 0, 0);
		case LEFT: return PBE::Vec3(0, 1, 0);
		case RIGHT: return PBE::Vec3(0, -1, 0);
	}
	return PBE::Vec3();
}

PBE::Vec3 GetBlockFaceNormal(BlockFace face)
{
	switch (face)
	{
		case BOTTOM: return PBE::Vec3(0.f, 0.f, -1.f);
		case TOP: return PBE::Vec3(0.f, 0.f, 1.f);
		case BACK: return PBE::Vec3(-1.f, 0.f, 0.f);
		case FRONT: return PBE::Vec3(1.f, 0.f, 0.f);
		case LEFT: return PBE::Vec3(0.f, 1.f, 0.f);
		case RIGHT: return PBE::Vec3(0.f, -1.f, 0.f);
	}
	return PBE::Vec3();
}

PBE::Vec3 GetBlockFaceTangent(BlockFace face)
{
	switch (face)
	{
		case BOTTOM: return PBE::Vec3(1.f, 0.f, 0.f);
		case TOP: return PBE::Vec3(1.f, 0.f, 0.f);
		case BACK: return PBE::Vec3(0.f, 1.f, 0.f);
		case FRONT: return PBE::Vec3(0.f, 1.f, 0.f);
		case LEFT: return PBE::Vec3(1.f, 0.f, 0.f);
		case RIGHT: return PBE::Vec3(1.f, 0.f, 0.f);
	}
	return PBE::Vec3();
}

PBE::Vec3 GetBlockFaceBitangent(BlockFace face)
{
	switch (face)
	{
		case BOTTOM: return PBE::Vec3(0.f, 1.f, 0.f);
		case TOP: return PBE::Vec3(0.f, 1.f, 0.f);
		case BACK: return PBE::Vec3(0.f, 0.f, 1.f);
		case FRONT: return PBE::Vec3(0.f, 0.f, 1.f);
		case LEFT: return PBE::Vec3(0.f, 0.f, 1.f);
		case RIGHT: return PBE::Vec3(0.f, 0.f, 1.f);
	}
	return PBE::Vec3();
}

BlockFace GetOppositeFace(BlockFace face)
{
	switch (face)
	{
		case BOTTOM: return TOP;
		case TOP: return BOTTOM;
		case BACK: return FRONT;
		case FRONT: return BACK;
		case LEFT: return RIGHT;
		case RIGHT: return LEFT;
	}
	return BlockFace();
}

void RegisterBlockDef(std::string_view name, BlockDef const& def)
{
	g_BlockDefs.push_back(def);
	BlockTypeID blockID =(BlockTypeID)(g_BlockDefs.size() - 1);
	g_BlockTypeMap[name.data()] = blockID;

	g_BlockBrokenEvents.push_back({});
	g_BlockPlacedEvents.push_back({});

	g_BlockBrokenEvents[blockID].AddSubscriber([blockID](PBE::IntVec3 blockPos) 
		{ 
			if (g_BlockDefs[blockID].m_BreakSound == nullptr)
				return false;
			PBE::SoundPlaybackConfig sndConfig;
			sndConfig.m_volume = 0.1f;
			sndConfig.m_position = PBE::Vec3(blockPos) + PBE::Vec3(0.5f);
			sndConfig.m_looping = false;
			
			g_Engine->m_pAudioSystem->PlaySound3D(g_BlockDefs[blockID].m_BreakSound,sndConfig, "Block");
			return false; 
		}
	);
	g_BlockPlacedEvents[blockID].AddSubscriber([blockID](PBE::IntVec3 blockPos) 
		{ 
			if (g_BlockDefs[blockID].m_PlaceSound == nullptr)
				return false;
			PBE::SoundPlaybackConfig sndConfig;
			sndConfig.m_volume = 0.1f;
			sndConfig.m_position = PBE::Vec3(blockPos) + PBE::Vec3(0.5f);
			sndConfig.m_looping = false;

			g_Engine->m_pAudioSystem->PlaySound3D(g_BlockDefs[blockID].m_PlaceSound, sndConfig, "Block");
			return false; 
		}
	);
}

int GetBlockTypeCount()
{
	return (int)g_BlockDefs.size();
}

std::unordered_map<std::string, BlockTypeID> g_BlockTypeMap;
std::vector<BlockDef> g_BlockDefs;
std::vector< PBE::EventDispatcher<PBE::IntVec3> > g_BlockBrokenEvents;
std::vector< PBE::EventDispatcher<PBE::IntVec3> > g_BlockPlacedEvents;