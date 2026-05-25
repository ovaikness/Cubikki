#include "Cubikki/Blocks/WorldGen/FlatWorldGenerator.hpp"
#include "Cubikki/Blocks/Chunk.hpp"

#include "Engine/Math/MathUtils.hpp"

FlatWorldGenerator::FlatWorldGenerator()
{
	m_HeightNoise->SetSource(fndSimplex);
	m_HeightNoise->SetOctaveCount(5);
	m_HeightNoise->SetLacunarity(2.0f);
	m_HeightNoise->SetGain(0.5f);

	m_RiverNoise->SetSource(fndSimplex);
	m_RiverNoise->SetOctaveCount(1);
	m_RiverNoise->SetLacunarity(2.0f);
	m_RiverNoise->SetGain(0.5f);

	m_ContinentalNoise->SetSource(fndSimplex);
	m_ContinentalNoise->SetOctaveCount(1);
	m_ContinentalNoise->SetLacunarity(2.0f);
	m_ContinentalNoise->SetGain(0.5f);

	m_PullNoise->SetSource(fndSimplex);
	m_PullNoise->SetOctaveCount(2);
	m_PullNoise->SetLacunarity(2.0f);
	m_PullNoise->SetGain(0.5f);

	m_ButteNoise->SetDistanceFunction(FastNoise::DistanceFunction::EuclideanSquared);
	m_ButteNoise->SetJitterModifier(0.5);

	m_SpacialDensityNoise->SetSource(fndSimplex);
	m_SpacialDensityNoise->SetOctaveCount(3);
	m_SpacialDensityNoise->SetLacunarity(2.0f);
	m_SpacialDensityNoise->SetGain(0.5f);
}

FlatWorldGenerator::~FlatWorldGenerator()
{
}

void FlatWorldGenerator::GenerateChunk(Chunk* chunk)
{
	Block air = { 0, 0 };
	Block grass = { 0, 1 };
	Block dirt = { 0, 3 };
	Block water = { 0 , 4 };
	Block sand = { 0, 5 };

	PBE::IntVec3 chunkBlockCoords{
		chunk->m_ChunkCoords.x * CHUNK_X_SIZE,
		chunk->m_ChunkCoords.y * CHUNK_Y_SIZE,
		chunk->m_ChunkCoords.z * CHUNK_Z_SIZE
	};

	if (chunkBlockCoords.z > 364.f)
	{
		return;
	}

	constexpr int PULL_PADDING = 8;
	constexpr int PADDED_CHUNK_X_SIZE = CHUNK_X_SIZE + PULL_PADDING * 2;
	constexpr int PADDED_CHUNK_Y_SIZE = CHUNK_Y_SIZE + PULL_PADDING * 2;

	constexpr int VOL_PADDING = 2;
	constexpr int PADDED_VOL_CHUNK_X_SIZE = CHUNK_X_SIZE + VOL_PADDING;
	constexpr int PADDED_VOL_CHUNK_Y_SIZE = CHUNK_Y_SIZE + VOL_PADDING;
	constexpr int PADDED_VOL_CHUNK_Z_SIZE = CHUNK_Z_SIZE + VOL_PADDING;

	float heightNoise[PADDED_CHUNK_X_SIZE * PADDED_CHUNK_Y_SIZE];
	float riverNoise[PADDED_CHUNK_X_SIZE * PADDED_CHUNK_Y_SIZE];
	float continentalNoise[PADDED_CHUNK_X_SIZE * PADDED_CHUNK_Y_SIZE];
	float butteNoise[PADDED_CHUNK_X_SIZE * PADDED_CHUNK_Y_SIZE];

	m_HeightNoise->GenUniformGrid2D(heightNoise, chunkBlockCoords.x - PULL_PADDING, chunkBlockCoords.y - PULL_PADDING, CHUNK_X_SIZE + PULL_PADDING * 2, CHUNK_Y_SIZE + PULL_PADDING * 2, 0.003f, 0);
	m_ButteNoise->GenUniformGrid2D(butteNoise, chunkBlockCoords.x - PULL_PADDING, chunkBlockCoords.y - PULL_PADDING, CHUNK_X_SIZE + PULL_PADDING * 2, CHUNK_Y_SIZE + PULL_PADDING * 2, 0.00015f, 1);
	m_RiverNoise->GenUniformGrid2D(riverNoise, chunkBlockCoords.x - PULL_PADDING, chunkBlockCoords.y - PULL_PADDING, CHUNK_X_SIZE + PULL_PADDING * 2, CHUNK_Y_SIZE + PULL_PADDING * 2, 0.0015f, 1);
	m_ContinentalNoise->GenUniformGrid2D(continentalNoise, chunkBlockCoords.x - PULL_PADDING, chunkBlockCoords.y - PULL_PADDING, CHUNK_X_SIZE + PULL_PADDING * 2, CHUNK_Y_SIZE + PULL_PADDING * 2, 0.0005f, 2);

	float pullXNoise[CHUNK_X_SIZE * CHUNK_Y_SIZE * CHUNK_Z_SIZE];
	float pullYNoise[CHUNK_X_SIZE * CHUNK_Y_SIZE * CHUNK_Z_SIZE];

	m_PullNoise->GenUniformGrid3D(pullXNoise, chunkBlockCoords.x, chunkBlockCoords.y, chunkBlockCoords.z, CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE, 0.01f, 3);
	m_PullNoise->GenUniformGrid3D(pullYNoise, chunkBlockCoords.x, chunkBlockCoords.y, chunkBlockCoords.z, CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE, 0.01f, 4);

	constexpr float RIVER_MAX_DEPTH = 6.f;
	constexpr float ADDED_BUTTE_HEIGHT = 1000.f;
	constexpr float SQUASH_FACTOR = 2.5f;
	constexpr float LOW_DENSITY_THRESHOLD = 0.7f;
	constexpr float HIGH_DENSITY_THRESHOLD = 0.95f;
	constexpr float LOW_DENSITY_Z = 0.f;
	constexpr float HIGH_DENSITY_Z = 64.f;
	constexpr float DENSITY_FREQUENCY = 0.0005f;

	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_COUNT; ++blockIndex)
	{
		PBE::IntVec3 localBlockCoords = GetLocalBlockCoordsFromBlockIndex(blockIndex);
		PBE::IntVec3 blockCoords = localBlockCoords + chunkBlockCoords;

		int x = blockCoords.x;
		int y = blockCoords.y;
		int z = blockCoords.z;

		float pullX = PBE::RangeMap(pullXNoise[blockIndex], -1.f, 1.f, -(float)PULL_PADDING, (float)PULL_PADDING);
		float pullY = PBE::RangeMap(pullYNoise[blockIndex], -1.f, 1.f, -(float)PULL_PADDING, (float)PULL_PADDING);

		int localWarpedX = localBlockCoords.x + (int)pullX + PULL_PADDING;
		int localWarpedY = localBlockCoords.y + (int)pullY + PULL_PADDING;

		int warpedX = localWarpedX + chunkBlockCoords.x;
		int warpedY = localWarpedY + chunkBlockCoords.y;

		float heightNoiseValue = heightNoise[localWarpedX + PADDED_CHUNK_X_SIZE * localWarpedY] * 0.5f + 0.5f;
		float butteNoiseValue = butteNoise[localWarpedX + PADDED_CHUNK_X_SIZE * localWarpedY] * 0.5f + 0.5f;

		float riverNoiseValue = std::abs(riverNoise[localWarpedX + PADDED_CHUNK_X_SIZE * localWarpedY]);

		float continentalNoiseValue = continentalNoise[localWarpedX + PADDED_CHUNK_X_SIZE * localWarpedY] * 0.5f + 0.5f;
		float continentalT = continentalNoiseValue * continentalNoiseValue * continentalNoiseValue;

		float scale = 5.f + 120.f * continentalT;
		float heightValue = heightNoiseValue * scale;

		if (butteNoiseValue > 0.8f)
		{
			float butteT = (butteNoiseValue - 0.8f) * 5.f;
			float butteHeight = ADDED_BUTTE_HEIGHT * butteT;
			heightValue += butteHeight;
		}

		bool isBeach = false;
		if (riverNoiseValue < 0.2f)
		{
			float normalizedRiverness = riverNoiseValue / 0.2f;
			float riverT = 1.f - normalizedRiverness * normalizedRiverness * normalizedRiverness;
			float riverBumps = heightNoise[localBlockCoords.x + CHUNK_X_SIZE * localBlockCoords.y] * 2.f;
			float threshold = PBE::RangeMap(0.8f, 0.f, 1.f, heightValue, -RIVER_MAX_DEPTH + riverBumps);
			heightValue = PBE::RangeMap(riverT, 0.f, 1.f, heightValue, -RIVER_MAX_DEPTH + riverBumps);
			if (heightValue < threshold)
			{
				isBeach = true;
			}
		}
		constexpr int BEACH_THICKNESS = 4; // how many voxels of sand below surface

		int surfaceZ = (int)std::floor(heightValue);
		int topBlockZ = (int)std::ceil(heightValue);

		if (blockCoords.z == topBlockZ)
		{
			// Surface block
			if (isBeach) chunk->m_BlockStorage.SetBlock(blockIndex, sand);
			else         chunk->m_BlockStorage.SetBlock(blockIndex, grass);
		}
		else if (blockCoords.z <= surfaceZ)
		{
			// Sub-surface
			int depthBelowTop = topBlockZ - blockCoords.z; // 1 for first block below surface

			if (isBeach && depthBelowTop <= BEACH_THICKNESS)
			{
				chunk->m_BlockStorage.SetBlock(blockIndex, sand);
			}
			else
			{
				// you can swap to 'stone' at some deeper threshold if you have it
				chunk->m_BlockStorage.SetBlock(blockIndex, dirt);
			}
		}
		else
		{
			// Above terrain: fill water up to sea level
			if (blockCoords.z <= 0)
			{
				chunk->m_BlockStorage.SetBlock(blockIndex, water);
			}
		}
	}

	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_COUNT; ++blockIndex)
	{
		PBE::IntVec3 localBlockCoords = GetLocalBlockCoordsFromBlockIndex(blockIndex);
		PBE::IntVec3 blockCoords = localBlockCoords + chunkBlockCoords;

		int x = blockCoords.x;
		int y = blockCoords.y;
		int z = blockCoords.z;

		float pullX = PBE::RangeMap(pullXNoise[blockIndex], -1.f, 1.f, -(float)PULL_PADDING, (float)PULL_PADDING);
		float pullY = PBE::RangeMap(pullYNoise[blockIndex], -1.f, 1.f, -(float)PULL_PADDING, (float)PULL_PADDING);

		int localWarpedX = localBlockCoords.x + (int)pullX + PULL_PADDING;
		int localWarpedY = localBlockCoords.y + (int)pullY + PULL_PADDING;

		int warpedX = localWarpedX + chunkBlockCoords.x;
		int warpedY = localWarpedY + chunkBlockCoords.y;

		float density = (m_SpacialDensityNoise->GenSingle3D((float)warpedX * DENSITY_FREQUENCY, (float)warpedY * DENSITY_FREQUENCY, (float)z * SQUASH_FACTOR * DENSITY_FREQUENCY, 6) * 0.5f) + 0.5f;
		float densityThreshold = PBE::RangeMapClamped((float)z, LOW_DENSITY_Z, HIGH_DENSITY_Z, LOW_DENSITY_THRESHOLD, HIGH_DENSITY_THRESHOLD);

		if (density > densityThreshold)
		{
			chunk->m_BlockStorage.SetBlock(blockIndex, air);
		}
	}
}


