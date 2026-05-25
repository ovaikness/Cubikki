#pragma once
#include "Cubikki/Common.hpp"

#include "Cubikki/Blocks/BlockCommon.hpp"
#include "Cubikki/Blocks/BlockStorage.hpp"
#include "Cubikki/Blocks/BlockLight.hpp"
#include "Cubikki/Blocks/BlockIterator.hpp"

#include "Cubikki/Net/CubikkiClient.hpp"

#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"

#include "Engine/Threads/atomic_vector.hpp"
#include "Engine/Threads/JobSystem.hpp"

#include "Engine/Graphics/Rgba8.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/Mesh.hpp"

#include "Engine/Net/NetConnection.hpp"

#include <atomic>
#include <vector>
#include <set>

enum class ChunkState : uint8_t
{
	INACTIVE,
	LOADING,
	SAVING,
	GENERATING_BLOCKS,
	INITIALIZING_LIGHTING,
	BUILDING_LIGHTING,
	GENERATING_CPUMESH,
	GENERATING_GPUMESH,
	COMPLETE
};

class BlockWorld;
class ChunkRegion;

class ChunkTryLoadJob;
class ChunkGenerateJob;
class ChunkGenerateMeshJob;
class ChunkGenerateLightingJob;
class ChunkSaveJob;

class Chunk
{
public:
	PBE::AABB3 m_BoundingBox;
	PBE::IntVec3 m_ChunkCoords;
	bool m_NeedsSaving = false;
	bool m_Active = false;

	std::mutex m_ChunkRecieveMutex;
	std::atomic<bool> m_HasBlockData = false;
	std::atomic<bool> m_MeshDirty = false;
	std::atomic<bool> m_ProcessingLighting = false;
	std::atomic<ChunkState> m_State = ChunkState::INACTIVE;

	BlockLight m_Lights[CHUNK_BLOCK_COUNT]{0};
	char m_BlockStates[CHUNK_BLOCK_COUNT]{0};

	int m_DirtyLightBufferFrameIndex{ 0 };
	std::vector<BlockIterator> m_DirtyLightSwapBuffers[2];

	BlockStorage m_BlockStorage;

	PBE::CPUMesh m_CPUMesh;
	PBE::CPUMesh m_TranslucentCPUMesh;

	std::vector<PBE::GPUMesh*> m_GPUMeshesToFree{};
	std::vector<PBE::GPUMesh*> m_TranslucentGPUMeshesToFree{};

	std::array<PBE::GPUMesh*, PBE::MAX_FRAMES_IN_FLIGHT> m_GPUMeshesInFlight{  };
	std::array<PBE::GPUMesh*, PBE::MAX_FRAMES_IN_FLIGHT> m_TranslucentGPUMeshesInFlight{};

	PBE::GPUMesh* m_GPUMesh;
	PBE::GPUMesh* m_TranslucentGPUMesh;

	BlockWorld* m_BlockWorld;

	Chunk* m_ChunkUp = nullptr;
	Chunk* m_ChunkDown = nullptr;
	Chunk* m_ChunkForward = nullptr;
	Chunk* m_ChunkBack = nullptr;
	Chunk* m_ChunkLeft = nullptr;
	Chunk* m_ChunkRight = nullptr;
	Chunk* m_ChunkUpForward = nullptr;
	Chunk* m_ChunkUpBack = nullptr;
	Chunk* m_ChunkUpLeft = nullptr;
	Chunk* m_ChunkUpRight = nullptr;
	Chunk* m_ChunkDownForward = nullptr;
	Chunk* m_ChunkDownBack = nullptr;
	Chunk* m_ChunkDownLeft = nullptr;
	Chunk* m_ChunkDownRight = nullptr;
	Chunk* m_ChunkForwardLeft = nullptr;
	Chunk* m_ChunkForwardRight = nullptr;
	Chunk* m_ChunkBackLeft = nullptr;
	Chunk* m_ChunkBackRight = nullptr;

	ChunkTryLoadJob* m_TryLoadJob = nullptr;
	ChunkGenerateJob* m_GenerateJob = nullptr;
	ChunkGenerateMeshJob* m_GenerateMeshJob = nullptr;
	ChunkSaveJob* m_SaveJob = nullptr;
public:
	Chunk(PBE::IntVec3 const& chunkCoords, BlockWorld* world);
	~Chunk();

	void Reset(PBE::IntVec3 const& chunkCoords, BlockWorld* world);

	void WaitForJobsToComplete();
	void ClaimJobs();
	void CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex);
	void Update();
	void GameUpdate();

	void Activate();
	void Deactivate();

	void Generate();

	void MarkBlockLightDirty(BlockIterator const& blockItr);
	bool HasDirtyLighting() const;
	void ProcessDirtyBlockLight(BlockIterator const& blockItr);
	void ProcessDirtyLighting();
	void SwapDirtyLightBuffers();
	void CreateAndMarkInitialLighting();

	void GenerateMesh();
	void AddVertsForBlockFace(
		PBE::Vec3 const& p0,
		PBE::Vec3 const& p1,
		PBE::Vec3 const& p2,
		PBE::Vec3 const& p3,
		PBE::Rgba8 const& color0,
		PBE::Rgba8 const& color1,
		PBE::Rgba8 const& color2,
		PBE::Rgba8 const& color3,
		PBE::Vec3 const& normal,
		PBE::Vec3 const& tangent,
		PBE::Vec3 const& bitangent,
		PBE::AABB2 const& texCoords,
		bool flipTris = false,
		bool isOpaque = true
	);

	void EvaluateColorsFromBlockIterators(
		BlockIterator const& ci,
		BlockIterator const& ri,
		BlockIterator const& li,
		BlockIterator const& tci,
		BlockIterator const& tri,
		BlockIterator const& tli,
		BlockIterator const& bci,
		BlockIterator const& bri,
		BlockIterator const& bli,
		PBE::Rgba8& color0,
		PBE::Rgba8& color1,
		PBE::Rgba8& color2,
		PBE::Rgba8& color3,
		bool& flip
	) const;

	void AppendBlockMesh(BlockIterator const& blockItr);

	void CmdDraw(PBE::Renderer* renderer,VkCommandBuffer cmd, uint32_t frameIndex);
	void CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex);

	void TryLoad(ChunkRegion* region);
	void Save();

	void SendChunkToClient(PBE::NetConnection<CubikkiMessageType>& client);
	void RecieveChunkFromServer(PBE::NetMessage<CubikkiMessageType>& client);

	void AddNeighborUp(Chunk* chunk);
	void AddNeighborDown(Chunk* chunk);
	void AddNeighborForward(Chunk* chunk);
	void AddNeighborBack(Chunk* chunk);
	void AddNeighborLeft(Chunk* chunk);
	void AddNeighborRight(Chunk* chunk);
	void AddNeighborUpForward(Chunk* chunk);
	void AddNeighborUpBack(Chunk* chunk);
	void AddNeighborUpLeft(Chunk* chunk);
	void AddNeighborUpRight(Chunk* chunk);
	void AddNeighborDownForward(Chunk* chunk);
	void AddNeighborDownBack(Chunk* chunk);
	void AddNeighborDownLeft(Chunk* chunk);
	void AddNeighborDownRight(Chunk* chunk);
	void AddNeighborForwardLeft(Chunk* chunk);
	void AddNeighborForwardRight(Chunk* chunk);
	void AddNeighborBackLeft(Chunk* chunk);
	void AddNeighborBackRight(Chunk* chunk);

	void SetBlock(PBE::IntVec3 const& localPos, Block const& block);
	Block GetBlock(PBE::IntVec3 const& localPos) const;

	void RemoveNeighborUp();
	void RemoveNeighborDown();
	void RemoveNeighborForward();
	void RemoveNeighborBack();
	void RemoveNeighborLeft();
	void RemoveNeighborRight();
	void RemoveNeighborUpForward();
	void RemoveNeighborUpBack();
	void RemoveNeighborUpLeft();
	void RemoveNeighborUpRight();
	void RemoveNeighborDownForward();
	void RemoveNeighborDownBack();
	void RemoveNeighborDownLeft();
	void RemoveNeighborDownRight();
	void RemoveNeighborForwardLeft();
	void RemoveNeighborForwardRight();
	void RemoveNeighborBackLeft();
	void RemoveNeighborBackRight();

	std::array<Chunk*, 26> GetBoxingNeighbors() const;
	bool AllBoxingNeighborsHaveBlockData() const;
	bool AllBoxingNeighborsHaveLightData() const;
};

class ChunkTryLoadJob : public PBE::Job
{
public:
	Chunk* m_Chunk;
public:
	ChunkTryLoadJob(Chunk* chunk)
		: m_Chunk(chunk)
	{
	}
	void Execute() override;
};

class ChunkGenerateJob : public PBE::Job
{
public:
	Chunk* m_Chunk;

public:
	ChunkGenerateJob(Chunk* chunk)
		: m_Chunk(chunk)
	{
	}
	void Execute() override
	{
		m_Chunk->Generate();
	}
};

class ChunkGenerateMeshJob : public PBE::Job
{
public:
	Chunk* m_Chunk;
public:
	ChunkGenerateMeshJob(Chunk* chunk)
		: m_Chunk(chunk)
	{
	}
	void Execute() override
	{
		m_Chunk->GenerateMesh();
	}
};

class ChunkGenerateLightingJob : public PBE::Job
{
public:
	Chunk* m_Chunk;
public:
	ChunkGenerateLightingJob(Chunk* chunk)
		: m_Chunk(chunk)
	{
	}
	void Execute() override
	{
		m_Chunk->ProcessDirtyLighting();
	}
};

class ChunkSaveJob : public PBE::Job
{
public:
	Chunk* m_Chunk;
public:
	ChunkSaveJob(Chunk* chunk)
		: m_Chunk(chunk)
	{
	}
	void Execute() override
	{
		m_Chunk->Save();
	}
};