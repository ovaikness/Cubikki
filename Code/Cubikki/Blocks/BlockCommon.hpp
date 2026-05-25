#pragma once

#include "Cubikki/Blocks/BlockDef.hpp"

#include "Engine/Core/EventDispatcher.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Vec3.hpp"

#include <vector>
#include <unordered_map>

enum BlockFace
{
	TOP,
	BOTTOM,
	FRONT,
	BACK,
	LEFT,
	RIGHT,
	COUNT
};

constexpr int CHUNK_X_BITS = 5;
constexpr int CHUNK_Y_BITS = 5;
constexpr int CHUNK_Z_BITS = 5;

constexpr int CHUNK_X_SIZE = 1 << CHUNK_X_BITS;
constexpr int CHUNK_Y_SIZE = 1 << CHUNK_Y_BITS;
constexpr int CHUNK_Z_SIZE = 1 << CHUNK_Z_BITS;

constexpr int CHUNK_X_SHIFT = 0;
constexpr int CHUNK_Y_SHIFT = CHUNK_X_BITS;
constexpr int CHUNK_Z_SHIFT = CHUNK_X_BITS + CHUNK_Y_BITS;

constexpr int CHUNK_X_MASK = CHUNK_X_SIZE - 1;
constexpr int CHUNK_Y_MASK = CHUNK_Y_SIZE - 1;
constexpr int CHUNK_Z_MASK = CHUNK_Z_SIZE - 1;

constexpr int CHUNK_BLOCK_COUNT = CHUNK_X_SIZE * CHUNK_Y_SIZE * CHUNK_Z_SIZE;

constexpr int CHUNK_REGION_X_BITS = 4;
constexpr int CHUNK_REGION_Y_BITS = 4;
constexpr int CHUNK_REGION_Z_BITS = 4;

constexpr int CHUNK_REGION_X_SIZE = 1 << CHUNK_REGION_X_BITS;
constexpr int CHUNK_REGION_Y_SIZE = 1 << CHUNK_REGION_Y_BITS;
constexpr int CHUNK_REGION_Z_SIZE = 1 << CHUNK_REGION_Z_BITS;

constexpr int CHUNK_REGION_X_SHIFT = 0;
constexpr int CHUNK_REGION_Y_SHIFT = CHUNK_REGION_X_BITS;
constexpr int CHUNK_REGION_Z_SHIFT = CHUNK_REGION_X_BITS + CHUNK_REGION_Y_BITS;

constexpr int CHUNK_REGION_X_MASK = CHUNK_REGION_X_SIZE - 1;
constexpr int CHUNK_REGION_Y_MASK = CHUNK_REGION_Y_SIZE - 1;
constexpr int CHUNK_REGION_Z_MASK = CHUNK_REGION_Z_SIZE - 1;

constexpr int CHUNK_REGION_CHUNK_COUNT = CHUNK_REGION_X_SIZE * CHUNK_REGION_Y_SIZE * CHUNK_REGION_Z_SIZE;

int GetLocalBlockXFromBlockIndex(int blockIndex);
int GetLocalBlockYFromBlockIndex(int blockIndex);
int GetLocalBlockZFromBlockIndex(int blockIndex);

int SetLocalBlockXInBlockIndex(int blockIndex, int x);
int SetLocalBlockYInBlockIndex(int blockIndex, int y);
int SetLocalBlockZInBlockIndex(int blockIndex, int z);

PBE::IntVec3 GetLocalBlockCoordsFromGlobalBlockCoords(PBE::IntVec3 blockCoords);
PBE::IntVec3 GetChunkCoordsFromBlockCoords(PBE::IntVec3 blockCoords);
PBE::IntVec3 GetBlockCoordsFromChunkCoords(PBE::IntVec3 chunkCoords);

PBE::IntVec3 GetLocalBlockCoordsFromBlockIndex(int blockIndex);
int GetBlockIndexFromLocalBlockCoords(PBE::IntVec3 blockCoords);

PBE::IntVec3 GetLocalChunkCoordsFromGlobalChunkCoords(PBE::IntVec3 chunkCoords);
int GetChunkIndexFromGlobalChunkCoords(PBE::IntVec3 chunkCoords);
int GetChunkIndexFromLocalChunkCoords(PBE::IntVec3 chunkCoords);

PBE::IntVec3 GetRegionCoordsFromGlobalChunkCoords(PBE::IntVec3 chunkCoords);
PBE::IntVec3 GetChunkCoordsFromRegionCoords(PBE::IntVec3 regionCoords);

PBE::Vec3 GetBlockFaceOffset(BlockFace face);
PBE::Vec3 GetBlockFaceNormal(BlockFace face);
PBE::Vec3 GetBlockFaceTangent(BlockFace face);
PBE::Vec3 GetBlockFaceBitangent(BlockFace face);

BlockFace GetOppositeFace(BlockFace face);

using BlockTypeID = uint16_t;
constexpr BlockTypeID AIR_BLOCK_TYPE_ID = 0;

void RegisterBlockDef(std::string_view name, BlockDef const& def);
int GetBlockTypeCount();

extern std::unordered_map<std::string, BlockTypeID> g_BlockTypeMap;
extern std::vector< BlockDef > g_BlockDefs;
extern std::vector< PBE::EventDispatcher<PBE::IntVec3> > g_BlockBrokenEvents;
extern std::vector< PBE::EventDispatcher<PBE::IntVec3> > g_BlockPlacedEvents;