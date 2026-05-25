#include "Cubikki/Blocks/Chunk.hpp"

#include "Cubikki/Blocks/BlockCommon.hpp"
#include "Cubikki/Blocks/BlockIterator.hpp"
#include "Cubikki/Blocks/BlockWorld.hpp"
#include "Cubikki/Common.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

Chunk::Chunk(PBE::IntVec3 const& chunkCoords, BlockWorld* world)
	: m_ChunkCoords(chunkCoords)
	, m_BlockWorld(world)
	, m_ProcessingLighting(false)
	, m_Active(false)
	, m_State(ChunkState::INACTIVE)
	, m_ChunkUp(nullptr)
	, m_ChunkDown(nullptr)
	, m_ChunkForward(nullptr)
	, m_ChunkBack(nullptr)
	, m_ChunkLeft(nullptr)
	, m_ChunkRight(nullptr)
{
	m_BoundingBox = PBE::AABB3(
		PBE::Vec3((float)(m_ChunkCoords.x * CHUNK_X_SIZE), (float)(m_ChunkCoords.y * CHUNK_Y_SIZE), (float)(m_ChunkCoords.z * CHUNK_Z_SIZE)),
		PBE::Vec3((float)(m_ChunkCoords.x * CHUNK_X_SIZE + CHUNK_X_SIZE), (float)(m_ChunkCoords.y * CHUNK_Y_SIZE + CHUNK_Y_SIZE), (float)(m_ChunkCoords.z * CHUNK_Z_SIZE + CHUNK_Z_SIZE))
	);
}

Chunk::~Chunk()
{
	WaitForJobsToComplete();

	if (m_GPUMesh)
	{
		auto itr = std::find(m_GPUMeshesToFree.begin(), m_GPUMeshesToFree.end(), m_GPUMesh);
		if (itr == m_GPUMeshesToFree.end())
		{
			bool found = false;
			for (int i = 0; i < m_GPUMeshesToFree.size(); ++i)
			{
				if (m_GPUMeshesToFree[i] == nullptr)
				{
					m_GPUMeshesToFree[i] = m_GPUMesh;
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_GPUMeshesToFree.push_back(m_GPUMesh);
			}
		}
	}

	if (m_TranslucentGPUMesh)
	{
		auto itr = std::find(m_TranslucentGPUMeshesToFree.begin(), m_TranslucentGPUMeshesToFree.end(), m_TranslucentGPUMesh);

		if (itr == m_TranslucentGPUMeshesToFree.end())
		{
			bool found = false;
			for (int i = 0; i < m_TranslucentGPUMeshesToFree.size(); ++i)
			{
				if (m_TranslucentGPUMeshesToFree[i] == nullptr)
				{
					m_TranslucentGPUMeshesToFree[i] = m_TranslucentGPUMesh;
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_TranslucentGPUMeshesToFree.push_back(m_TranslucentGPUMesh);
			}
		}
	}

	for (PBE::GPUMesh* mesh : m_GPUMeshesToFree)
	{
		if (mesh)
		{
			g_Engine->m_pRenderer->FreeGPUMesh(mesh);
		}
	}

	for (PBE::GPUMesh* mesh : m_TranslucentGPUMeshesToFree)
	{
		if (mesh)
		{
			g_Engine->m_pRenderer->FreeGPUMesh(mesh);
		}
	}
}

void Chunk::Reset(PBE::IntVec3 const& chunkCoords, BlockWorld* world)
{
	WaitForJobsToComplete();
	m_HasBlockData = false;
	m_ChunkCoords = chunkCoords;
	m_BlockWorld = world;
	m_ProcessingLighting = false;
	m_Active = false;
	m_State = ChunkState::INACTIVE;
	m_ChunkUp = nullptr;
	m_ChunkDown = nullptr;
	m_ChunkForward = nullptr;
	m_ChunkBack = nullptr;
	m_ChunkLeft = nullptr;
	m_ChunkRight = nullptr;
	m_BoundingBox = PBE::AABB3(
		PBE::Vec3((float)(m_ChunkCoords.x * CHUNK_X_SIZE), (float)(m_ChunkCoords.y * CHUNK_Y_SIZE), (float)(m_ChunkCoords.z * CHUNK_Z_SIZE)),
		PBE::Vec3((float)(m_ChunkCoords.x * CHUNK_X_SIZE + CHUNK_X_SIZE), (float)(m_ChunkCoords.y * CHUNK_Y_SIZE + CHUNK_Y_SIZE), (float)(m_ChunkCoords.z * CHUNK_Z_SIZE + CHUNK_Z_SIZE))
	);

	if (m_GPUMesh)
	{
		auto itr = std::find(m_GPUMeshesToFree.begin(), m_GPUMeshesToFree.end(), m_GPUMesh);
		if (itr == m_GPUMeshesToFree.end())
		{
			bool found = false;
			for (int i = 0; i < m_GPUMeshesToFree.size(); ++i)
			{
				if (m_GPUMeshesToFree[i] == nullptr)
				{
					m_GPUMeshesToFree[i] = m_GPUMesh;
					found = true;
					break;
				}
			}
			if (!found)
			{
				m_GPUMeshesToFree.push_back(m_GPUMesh);
			}
		}
	}

	if (m_TranslucentGPUMesh)
	{
		auto itr = std::find(m_TranslucentGPUMeshesToFree.begin(), m_TranslucentGPUMeshesToFree.end(), m_TranslucentGPUMesh);
		if (itr == m_TranslucentGPUMeshesToFree.end())
		{
			bool found = false;
			for (int i = 0; i < m_TranslucentGPUMeshesToFree.size(); ++i)
			{
				if (m_TranslucentGPUMeshesToFree[i] == nullptr)
				{
					m_TranslucentGPUMeshesToFree[i] = m_TranslucentGPUMesh;
					found = true;
					break;
				}
			}
			if (!found)
			{
				m_TranslucentGPUMeshesToFree.push_back(m_TranslucentGPUMesh);
			}
		}
	}

	for (PBE::GPUMesh* mesh : m_GPUMeshesToFree)
	{
		if (mesh)
		{
			g_Engine->m_pRenderer->FreeGPUMesh(mesh);
		}
	}

	for (PBE::GPUMesh* mesh : m_TranslucentGPUMeshesToFree)
	{
		if (mesh)
		{
			g_Engine->m_pRenderer->FreeGPUMesh(mesh);
		}
	}

	m_TranslucentGPUMeshesToFree.clear();
	m_GPUMeshesToFree.clear();
	m_GPUMesh = nullptr;
	m_TranslucentGPUMesh = nullptr;
	m_CPUMesh = {};
	m_TranslucentCPUMesh = {};
	m_MeshDirty = false;
	m_DirtyLightBufferFrameIndex = 0;
	m_DirtyLightSwapBuffers[0].clear();
	m_DirtyLightSwapBuffers[1].clear();
	for (int i = 0; i < CHUNK_BLOCK_COUNT; ++i)
	{
		m_Lights[i].m_LightLevel = 0;
		m_BlockStates[i] = 0;
	}
	m_BlockStorage.Clear();
	m_TryLoadJob = nullptr;
	m_GenerateJob = nullptr;
	m_GenerateMeshJob = nullptr;
	m_SaveJob = nullptr;

	m_ChunkUp = nullptr;
	m_ChunkDown = nullptr;
	m_ChunkForward = nullptr;
	m_ChunkBack = nullptr;
	m_ChunkLeft = nullptr;
	m_ChunkRight = nullptr;
	m_ChunkUpForward = nullptr;
	m_ChunkUpBack = nullptr;
	m_ChunkUpLeft = nullptr;
	m_ChunkUpRight = nullptr;
	m_ChunkDownForward = nullptr;
	m_ChunkDownBack = nullptr;
	m_ChunkDownLeft = nullptr;
	m_ChunkDownRight = nullptr;
	m_ChunkForwardLeft = nullptr;
	m_ChunkForwardRight = nullptr;
	m_ChunkBackLeft = nullptr;
	m_ChunkBackRight = nullptr;
}

void Chunk::WaitForJobsToComplete()
{
	if (m_TryLoadJob)
	{
		g_Engine->m_pJobSystem->WaitUntilAndClaimJobIfComplete(m_TryLoadJob);
		delete m_TryLoadJob;
		m_TryLoadJob = nullptr;
	}

	if (m_GenerateJob)
	{
		g_Engine->m_pJobSystem->WaitUntilAndClaimJobIfComplete(m_GenerateJob);
		delete m_GenerateJob;
		m_GenerateJob = nullptr;
	}

	if (m_GenerateMeshJob)
	{
		g_Engine->m_pJobSystem->WaitUntilAndClaimJobIfComplete(m_GenerateMeshJob);
		delete m_GenerateMeshJob;
		m_GenerateMeshJob = nullptr;
	}

	if (m_SaveJob)
	{
		g_Engine->m_pJobSystem->WaitUntilAndClaimJobIfComplete(m_SaveJob);
		delete m_SaveJob;
		m_SaveJob = nullptr;
	}
}

void Chunk::ClaimJobs()
{
	if (m_TryLoadJob)
	{
		if (g_Engine->m_pJobSystem->ClaimJobIfComplete(m_TryLoadJob))
		{
			delete m_TryLoadJob;
			m_TryLoadJob = nullptr;
		}
	}

	if (m_GenerateJob)
	{
		if (g_Engine->m_pJobSystem->ClaimJobIfComplete(m_GenerateJob))
		{
			delete m_GenerateJob;
			m_GenerateJob = nullptr;
		}
	}

	if (m_GenerateMeshJob)
	{
		if (g_Engine->m_pJobSystem->ClaimJobIfComplete(m_GenerateMeshJob))
		{
			delete m_GenerateMeshJob;
			m_GenerateMeshJob = nullptr;
		}
	}

	if (m_SaveJob)
	{
		if (g_Engine->m_pJobSystem->ClaimJobIfComplete(m_SaveJob))
		{
			delete m_SaveJob;
			m_SaveJob = nullptr;
		}
	}
}

void Chunk::CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	if (m_State != ChunkState::GENERATING_GPUMESH)
	{
		return;
	}

	if (m_GPUMesh)
	{
		auto itr = std::find(m_GPUMeshesToFree.begin(), m_GPUMeshesToFree.end(), m_GPUMesh);
		if (itr == m_GPUMeshesToFree.end())
		{
			bool found = false;
			for (int i = 0; i < m_GPUMeshesToFree.size(); ++i)
			{
				if (m_GPUMeshesToFree[i] == nullptr)
				{
					m_GPUMeshesToFree[i] = m_GPUMesh;
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_GPUMeshesToFree.push_back(m_GPUMesh);
			}
		}
	}

	if (m_TranslucentGPUMesh)
	{
		auto itr = std::find(m_TranslucentGPUMeshesToFree.begin(), m_TranslucentGPUMeshesToFree.end(), m_TranslucentGPUMesh);
		if (itr == m_TranslucentGPUMeshesToFree.end())
		{
			bool found = false;
			for (int i = 0; i < m_TranslucentGPUMeshesToFree.size(); ++i)
			{
				if (m_TranslucentGPUMeshesToFree[i] == nullptr)
				{
					m_TranslucentGPUMeshesToFree[i] = m_TranslucentGPUMesh;
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_TranslucentGPUMeshesToFree.push_back(m_TranslucentGPUMesh);
			}
		}
	}

	m_GPUMesh = g_Engine->m_pRenderer->CmdCreateMesh(cmd, frameIndex, m_CPUMesh, PBE::Stringf("Chunk %i-%i-%i",m_ChunkCoords.x,m_ChunkCoords.y,m_ChunkCoords.z));
	if (m_TranslucentCPUMesh.m_Positions->size() > 0)
	{
		m_TranslucentGPUMesh = g_Engine->m_pRenderer->CmdCreateMesh(cmd, frameIndex, m_TranslucentCPUMesh, PBE::Stringf("Trans Chunk %i-%i-%i", m_ChunkCoords.x, m_ChunkCoords.y, m_ChunkCoords.z));
	}
	m_MeshDirty = false;
	m_CPUMesh = {};
	m_TranslucentCPUMesh = {};
	m_State = ChunkState::COMPLETE;
}

void Chunk::Update()
{
	for (auto itr = m_GPUMeshesToFree.begin(); itr != m_GPUMeshesToFree.end(); ++itr)
	{
		PBE::GPUMesh* mesh = *itr;
		if (mesh)
		{
			auto itr2 = std::find(m_GPUMeshesInFlight.begin(), m_GPUMeshesInFlight.end(), mesh);
			if (itr2 == m_GPUMeshesInFlight.end())
			{
				g_Engine->m_pRenderer->FreeGPUMesh(mesh);
				(*itr) = nullptr;
			}
		}
	}
	for (auto itr = m_TranslucentGPUMeshesToFree.begin(); itr != m_TranslucentGPUMeshesToFree.end(); ++itr)
	{
		PBE::GPUMesh* mesh = *itr;
		if (mesh)
		{
			auto itr2 = std::find(m_TranslucentGPUMeshesInFlight.begin(), m_TranslucentGPUMeshesInFlight.end(), mesh);
			if (itr2 == m_TranslucentGPUMeshesInFlight.end())
			{
				g_Engine->m_pRenderer->FreeGPUMesh(mesh);
				(*itr) = nullptr;
			}
		}
	}
	ClaimJobs();
	switch (m_State)
	{
		case ChunkState::INACTIVE:
		{
			if (m_Active)
			{
				m_State = ChunkState::LOADING;
			}
			break;
		}

		case ChunkState::LOADING:
		{
			if (!m_TryLoadJob)
			{
				m_TryLoadJob = new ChunkTryLoadJob(this);
				g_Engine->m_pJobSystem->SubmitJob(m_TryLoadJob);
			}
			break;
		}

		case ChunkState::GENERATING_BLOCKS:
		{
			if (m_BlockWorld->m_NetMode != BlockWorldNetMode::SERVER)
			{
				return;
			}
			if (!m_GenerateJob)
			{
				m_GenerateJob = new ChunkGenerateJob(this);
				g_Engine->m_pJobSystem->SubmitJob(m_GenerateJob);
			}
			break;
		}

		case ChunkState::BUILDING_LIGHTING:
		{
			if (!HasDirtyLighting())
			{
				m_State = ChunkState::GENERATING_CPUMESH;
			}
			break;
		}

		case ChunkState::GENERATING_CPUMESH:
		{
			if (!m_GenerateMeshJob && AllBoxingNeighborsHaveLightData())
			{
				m_GenerateMeshJob = new ChunkGenerateMeshJob(this);
				g_Engine->m_pJobSystem->SubmitJob(m_GenerateMeshJob);
			}
			break;
		}

		case ChunkState::COMPLETE:
		{
			if (!m_Active)
			{
				m_State = ChunkState::INACTIVE;
			}
			else if (HasDirtyLighting())
			{
				m_State = ChunkState::BUILDING_LIGHTING;
			}
			else if (m_MeshDirty)
			{
				m_State = ChunkState::GENERATING_CPUMESH;
			}
			else
			{
				GameUpdate();
			}
			break;
		}

		case ChunkState::SAVING:
		{
			if (!m_NeedsSaving)
			{
				m_State = ChunkState::INACTIVE;
			}
			if (!m_SaveJob)
			{
				m_SaveJob = new ChunkSaveJob(this);
				g_Engine->m_pJobSystem->SubmitJob(m_SaveJob);
			}
			break;
		}
	}
}

void Chunk::GameUpdate()
{
	//if (((float)rand() / (float)RAND_MAX) < 0.1f)
	//{
	//	int index = rand() % CHUNK_BLOCK_COUNT;
	//	SetBlock(GetLocalBlockCoordsFromBlockIndex(index), { 0,0 });
	//}
}

void Chunk::Activate()
{
	m_Active = true;
}

void Chunk::Deactivate()
{
	m_Active = false;
	RemoveNeighborUp();
	RemoveNeighborDown();
	RemoveNeighborForward();
	RemoveNeighborBack();
	RemoveNeighborLeft();
	RemoveNeighborRight();
	RemoveNeighborUpForward();
	RemoveNeighborUpBack();
	RemoveNeighborUpLeft();
	RemoveNeighborUpRight();
	RemoveNeighborDownForward();
	RemoveNeighborDownBack();
	RemoveNeighborDownLeft();
	RemoveNeighborDownRight();
	RemoveNeighborForwardLeft();
	RemoveNeighborForwardRight();
	RemoveNeighborBackLeft();
	RemoveNeighborBackRight();

	if (m_GPUMesh)
	{
		auto itr = std::find(m_GPUMeshesToFree.begin(), m_GPUMeshesToFree.end(), m_GPUMesh);
		if (itr == m_GPUMeshesToFree.end())
		{
			bool found = false;
			for (int i = 0; i < m_GPUMeshesToFree.size(); ++i)
			{
				if (m_GPUMeshesToFree[i] == nullptr)
				{
					m_GPUMeshesToFree[i] = m_GPUMesh;
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_GPUMeshesToFree.push_back(m_GPUMesh);
			}
		}
	}
	m_GPUMesh = nullptr;

	m_State = ChunkState::SAVING;
}

void Chunk::MarkBlockLightDirty(BlockIterator const& blockItr)
{
	if (blockItr.m_pChunk == nullptr)
	{
		return;
	}

	if (blockItr.m_pChunk != this)
	{
		m_BlockWorld->MarkBlockLightDirty(blockItr);
		return;
	}

	int backBufferIndex = m_DirtyLightBufferFrameIndex == 0 ? 1 : 0;
	m_DirtyLightSwapBuffers[backBufferIndex].push_back(blockItr);
}

bool Chunk::HasDirtyLighting() const
{
	return m_DirtyLightSwapBuffers[0].size() > 0 || m_DirtyLightSwapBuffers[1].size() > 0;
}

void Chunk::ProcessDirtyBlockLight(BlockIterator const& blockItr)
{
	BlockLight* light = blockItr.GetLight();
	BlockDef* blockDef = blockItr.GetBlockDef();

	int blockLight = light->GetBlockLight();
	int newLightLevel = blockLight - 1;
	int blockLightLevel = blockDef->m_BlockLightIntensity;

	if (blockDef->m_IsOpaque)
	{
		newLightLevel = blockLightLevel;
	}
	else
	{
		if (blockLightLevel > newLightLevel)
		{
			newLightLevel = blockLightLevel;
		}

		// Cache neighbors and compute minimum light levels
		for (int i = 0; i < (int)BlockFace::COUNT; ++i)
		{
			BlockIterator neighbor = blockItr.GetNeighbor((BlockFace)i);
			if (neighbor.IsValid())
			{
				BlockLight* neighborLight = neighbor.GetLight();
				int neighborLightLevel = std::max(neighborLight->GetBlockLight() - 1, 0);
				if (neighborLightLevel > newLightLevel)
				{
					newLightLevel = neighborLightLevel;
				}
			}
		}
	}

	if (newLightLevel != blockLight)
	{
		light->SetBlockLight(newLightLevel);

		for (int i = 0; i < (int)BlockFace::COUNT; ++i)
		{
			BlockIterator neighbor = blockItr.GetNeighbor((BlockFace)i);
			BlockDef* neighborBlockDef = neighbor.GetBlockDef();
			if (neighbor.IsValid())
			{
				if (!neighborBlockDef->m_IsOpaque)
				{
					MarkBlockLightDirty(neighbor);
				}
			}
		}
	}
}

void Chunk::ProcessDirtyLighting()
{
	while (HasDirtyLighting())
	{
		for (BlockIterator const& blockItr : m_DirtyLightSwapBuffers[m_DirtyLightBufferFrameIndex])
		{
			if (blockItr.IsValid())
			{
				ProcessDirtyBlockLight(blockItr);
			}
		}

		SwapDirtyLightBuffers();
	}

	m_ProcessingLighting = false;
	m_MeshDirty = true;
	if (m_ChunkBack)
	{
		m_ChunkBack->m_MeshDirty = true;
	}
	if (m_ChunkDown)
	{
		m_ChunkDown->m_MeshDirty = true;
	}
	if (m_ChunkDownBack)
	{
		m_ChunkDownBack->m_MeshDirty = true;
	}
	if (m_ChunkDownForward)
	{
		m_ChunkDownForward->m_MeshDirty = true;
	}
	if (m_ChunkDownLeft)
	{
		m_ChunkDownLeft->m_MeshDirty = true;
	}
	if (m_ChunkDownRight)
	{
		m_ChunkDownRight->m_MeshDirty = true;
	}
	if (m_ChunkForward)
	{
		m_ChunkForward->m_MeshDirty = true;
	}
	if (m_ChunkForwardLeft)
	{
		m_ChunkForwardLeft->m_MeshDirty = true;
	}
	if (m_ChunkForwardRight)
	{
		m_ChunkForwardRight->m_MeshDirty = true;
	}
	if (m_ChunkBackLeft)
	{
		m_ChunkBackLeft->m_MeshDirty = true;
	}
	if (m_ChunkBackRight)
	{
		m_ChunkBackRight->m_MeshDirty = true;
	}
	if (m_ChunkLeft)
	{
		m_ChunkLeft->m_MeshDirty = true;
	}
	if (m_ChunkRight)
	{
		m_ChunkRight->m_MeshDirty = true;
	}
	if (m_ChunkUp)
	{
		m_ChunkUp->m_MeshDirty = true;
	}
	if (m_ChunkUpBack)
	{
		m_ChunkUpBack->m_MeshDirty = true;
	}
	if (m_ChunkUpForward)
	{
		m_ChunkUpForward->m_MeshDirty = true;
	}
	if (m_ChunkUpLeft)
	{
		m_ChunkUpLeft->m_MeshDirty = true;
	}
	if (m_ChunkUpRight)
	{
		m_ChunkUpRight->m_MeshDirty = true;
	}
}

void Chunk::SwapDirtyLightBuffers()
{
	m_DirtyLightSwapBuffers[m_DirtyLightBufferFrameIndex].clear();
	int backBufferIndex = m_DirtyLightBufferFrameIndex == 0 ? 1 : 0;
	m_DirtyLightBufferFrameIndex = backBufferIndex;
}

void Chunk::CreateAndMarkInitialLighting()
{
	Block entry = m_BlockStorage.GetBlock(0);
	
	if (entry.m_BlockCount == CHUNK_BLOCK_COUNT)
	{
		BlockDef const& def = g_BlockDefs[entry.m_BlockID];
		if (def.m_IsOpaque)
		{
			return;
		}
		else
		{
			for (BlockIterator itr = BlockIterator(this, 0); !itr.IsEnd(); ++itr)
			{
				int z = GetLocalBlockZFromBlockIndex(itr.m_Index);
				int x = GetLocalBlockXFromBlockIndex(itr.m_Index);
				int y = GetLocalBlockYFromBlockIndex(itr.m_Index);

				if (
					x == 0 ||
					x == CHUNK_X_SIZE - 1 ||
					y == 0 ||
					y == CHUNK_Y_SIZE - 1 ||
					z == 0 ||
					z == CHUNK_Z_SIZE - 1
					)
				{
					MarkBlockLightDirty(itr);
				}
			}
			return;
		}
	}

	for (BlockIterator itr = BlockIterator(this, 0); !itr.IsEnd(); ++itr)
	{
		BlockDef* blockDef = itr.GetBlockDef();

		int x = GetLocalBlockXFromBlockIndex(itr.m_Index);
		int y = GetLocalBlockYFromBlockIndex(itr.m_Index);
		int z = GetLocalBlockZFromBlockIndex(itr.m_Index);

		if (blockDef->m_BlockLightIntensity > 0)
		{
			MarkBlockLightDirty(itr);
			continue;
		}

		if (
			x == 0 ||
			x == CHUNK_X_SIZE - 1 ||
			y == 0 ||
			y == CHUNK_Y_SIZE - 1 ||
			z == 0 ||
			z == CHUNK_Z_SIZE - 1
			)
		{
			if (!blockDef->m_IsOpaque)
			{
				MarkBlockLightDirty(itr);
				continue;
			}
		}
	}
}

void Chunk::Generate()
{
	m_BlockWorld->m_WorldGenerator->GenerateChunk(this);
	CreateAndMarkInitialLighting();
	m_State = ChunkState::BUILDING_LIGHTING;
	m_HasBlockData = true;
}

void Chunk::GenerateMesh()
{
	m_CPUMesh = {};
	m_TranslucentCPUMesh = {};

	for (int i = 0; i < m_BlockStorage.m_BlockEntries.size(); ++i)
	{
		Block entry = m_BlockStorage.GetBlock(i);

		if (entry.m_BlockCount == CHUNK_BLOCK_COUNT)
		{
			BlockDef const& def = g_BlockDefs[entry.m_BlockID];
			if (!def.m_IsVisible)
			{
				m_State = ChunkState::COMPLETE;
				m_GPUMesh = nullptr;
				m_TranslucentGPUMesh = nullptr;
				return;
			}
		}
	}

	m_CPUMesh.InitializeArrays();
	m_TranslucentCPUMesh.InitializeArrays();

	for (BlockIterator itr = BlockIterator(this, 0); !itr.IsEnd(); ++itr)
	{
		BlockDef* blockDef = itr.GetBlockDef();
		if (blockDef->m_IsVisible)
		{
			AppendBlockMesh(itr);
		}
	}

	m_State = ChunkState::GENERATING_GPUMESH;
}

void Chunk::AddVertsForBlockFace(PBE::Vec3 const& p0, PBE::Vec3 const& p1, PBE::Vec3 const& p2, PBE::Vec3 const& p3, PBE::Rgba8 const& color0, PBE::Rgba8 const& color1, PBE::Rgba8 const& color2, PBE::Rgba8 const& color3, PBE::Vec3 const& normal, PBE::Vec3 const& tangent, PBE::Vec3 const& bitangent, PBE::AABB2 const& texCoords, bool flipTris, bool isOpaque)
{
	using namespace PBE;

	Vec2 minsUV = texCoords.m_Mins;
	Vec2 maxsUV = texCoords.m_Maxs;

	Vec2 uv0 = Vec2(minsUV.x, maxsUV.y);
	Vec2 uv1 = Vec2(minsUV.x, minsUV.y);
	Vec2 uv2 = Vec2(maxsUV.x, maxsUV.y);
	Vec2 uv3 = Vec2(maxsUV.x, minsUV.y);

	Vec3 e0 = p1 - p0;
	Vec3 e1 = p2 - p1;

	CPUMesh& mesh = isOpaque ? m_CPUMesh : m_TranslucentCPUMesh;

	mesh.m_Positions->push_back(p0);
	mesh.m_Positions->push_back(p1);
	mesh.m_Positions->push_back(p2);
	mesh.m_Positions->push_back(p3);

	mesh.m_Normals->push_back(normal);
	mesh.m_Normals->push_back(normal);
	mesh.m_Normals->push_back(normal);
	mesh.m_Normals->push_back(normal);

	mesh.m_Tangents->push_back(tangent);
	mesh.m_Tangents->push_back(tangent);
	mesh.m_Tangents->push_back(tangent);
	mesh.m_Tangents->push_back(tangent);

	mesh.m_Bitangents->push_back(bitangent);
	mesh.m_Bitangents->push_back(bitangent);
	mesh.m_Bitangents->push_back(bitangent);
	mesh.m_Bitangents->push_back(bitangent);

	mesh.m_BoneIndices->push_back({ -1, -1, -1, -1 });
	mesh.m_BoneIndices->push_back({ -1, -1, -1, -1 });
	mesh.m_BoneIndices->push_back({ -1, -1, -1, -1 });
	mesh.m_BoneIndices->push_back({ -1, -1, -1, -1 });

	mesh.m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	mesh.m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	mesh.m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });
	mesh.m_BoneWeights->push_back({ 1.f, 0.f, 0.f, 0.f });

	mesh.m_UVs->push_back(uv0);
	mesh.m_UVs->push_back(uv1);
	mesh.m_UVs->push_back(uv2);
	mesh.m_UVs->push_back(uv3);

	mesh.m_Colors->push_back(color0);
	mesh.m_Colors->push_back(color1);
	mesh.m_Colors->push_back(color2);
	mesh.m_Colors->push_back(color3);

	uint32_t startIndex = (uint32_t)mesh.m_Positions->size() - 4;

	if (flipTris)
	{
		mesh.m_Indices.push_back(startIndex + 3);
		mesh.m_Indices.push_back(startIndex + 0);
		mesh.m_Indices.push_back(startIndex + 1);

		mesh.m_Indices.push_back(startIndex + 0);
		mesh.m_Indices.push_back(startIndex + 3);
		mesh.m_Indices.push_back(startIndex + 2);
	}
	else
	{
		mesh.m_Indices.push_back(startIndex + 1);
		mesh.m_Indices.push_back(startIndex + 2);
		mesh.m_Indices.push_back(startIndex + 0);

		mesh.m_Indices.push_back(startIndex + 3);
		mesh.m_Indices.push_back(startIndex + 2);
		mesh.m_Indices.push_back(startIndex + 1);
	}
}

void Chunk::EvaluateColorsFromBlockIterators(BlockIterator const& ci, BlockIterator const& ri, BlockIterator const& li, BlockIterator const& tci, BlockIterator const& tri, BlockIterator const& tli, BlockIterator const& bci, BlockIterator const& bri, BlockIterator const& bli, PBE::Rgba8& color0, PBE::Rgba8& color1, PBE::Rgba8& color2, PBE::Rgba8& color3, bool& flip) const
{
	bool rIsOpaque = ri.GetBlockDef()->m_IsOpaque;
	bool lIsOpaque = li.GetBlockDef()->m_IsOpaque;

	bool tcIsOpaque = tci.GetBlockDef()->m_IsOpaque;
	bool trIsOpaque = tri.GetBlockDef()->m_IsOpaque;
	bool tlIsOpaque = tli.GetBlockDef()->m_IsOpaque;

	bool bcIsOpaque = bci.GetBlockDef()->m_IsOpaque;
	bool brIsOpaque = bri.GetBlockDef()->m_IsOpaque;
	bool blIsOpaque = bli.GetBlockDef()->m_IsOpaque;

	bool blocked;

	float c = ci.GetLight()->GetBlockLight() / 15.f;
	float r = ri.GetLight()->GetBlockLight() / 15.f;
	float l = li.GetLight()->GetBlockLight() / 15.f;

	float tc = tci.GetLight()->GetBlockLight() / 15.f;
	blocked = false || (rIsOpaque && tcIsOpaque);
	float tr = blocked ? 0 : tri.GetLight()->GetBlockLight() / 15.f;
	blocked = false || (lIsOpaque && tcIsOpaque);
	float tl = blocked ? 0 : tli.GetLight()->GetBlockLight() / 15.f;

	float bc = bci.GetLight()->GetBlockLight() / 15.f;
	blocked = false || (rIsOpaque && bcIsOpaque);
	float br = blocked ? 0 : bri.GetLight()->GetBlockLight() / 15.f;
	blocked = false || (lIsOpaque && bcIsOpaque);
	float bl = blocked ? 0 : bli.GetLight()->GetBlockLight() / 15.f;


	float p0Light = (bc + br + c + r) / 4.f;
	float p1Light = (tc + tr + c + r) / 4.f;
	float p2Light = (bc + bl + c + l) / 4.f;
	float p3Light = (tc + tl + c + l) / 4.f;

	float p0AO = 0.f;
	float p1AO = 0.f;
	float p2AO = 0.f;
	float p3AO = 0.f;

	for (int i = 0; i < 3; ++i)
	{
		switch (i)
		{
			case 0:
				p0AO += bcIsOpaque ? 0.f : 1.f;
				p1AO += tcIsOpaque ? 0.f : 1.f;
				p2AO += bcIsOpaque ? 0.f : 1.f;
				p3AO += tcIsOpaque ? 0.f : 1.f;
				break;
			case 1:
				p0AO += brIsOpaque ? 0.f : 1.f;
				p1AO += trIsOpaque ? 0.f : 1.f;
				p2AO += blIsOpaque ? 0.f : 1.f;
				p3AO += tlIsOpaque ? 0.f : 1.f;
				break;
			case 2:
				p0AO += rIsOpaque ? 0.f : 1.f;
				p1AO += rIsOpaque ? 0.f : 1.f;
				p2AO += lIsOpaque ? 0.f : 1.f;
				p3AO += lIsOpaque ? 0.f : 1.f;
				break;
		}
	}
	p0AO /= 3.f;
	p0AO = PBE::Max(p0AO, 0.6f);
	p1AO /= 3.f;
	p1AO = PBE::Max(p1AO, 0.6f);
	p2AO /= 3.f;
	p2AO = PBE::Max(p2AO, 0.6f);
	p3AO /= 3.f;
	p3AO = PBE::Max(p3AO, 0.6f);

	if (p0AO + p3AO > p1AO + p2AO)
	{
		flip = true;
	}
	else if (p0Light + p3Light < p1Light + p2Light)
	{
		flip = true;
	}

	color0.r = (uint8_t)(p0Light * 255.f);
	color1.r = (uint8_t)(p1Light * 255.f);
	color2.r = (uint8_t)(p2Light * 255.f);
	color3.r = (uint8_t)(p3Light * 255.f);

	color0.g = (uint8_t)(p0AO * 255.f);
	color1.g = (uint8_t)(p1AO * 255.f);
	color2.g = (uint8_t)(p2AO * 255.f);
	color3.g = (uint8_t)(p3AO * 255.f);
}

void Chunk::AppendBlockMesh(BlockIterator const& blockItr)
{
	bool isOpaque = blockItr.GetBlockDef()->m_IsOpaque;
	for (int i = 0; i < (int)BlockFace::COUNT; ++i)
	{
		BlockIterator neighborItr = blockItr.GetNeighbor((BlockFace)i);
		if (neighborItr.IsValid() && !neighborItr.GetBlockDef()->m_IsOpaque && neighborItr.GetBlockTypeID() != blockItr.GetBlockTypeID())
		{
			PBE::Vec3 blockPos = blockItr.GetLocalBlockPos();
			BlockLight* light = neighborItr.GetLight();

			PBE::Vec3 p0, p1, p2, p3;
			bool flip = false;
			PBE::Rgba8 color0 = PBE::Rgba8::BLACK;
			PBE::Rgba8 color1 = PBE::Rgba8::BLACK;
			PBE::Rgba8 color2 = PBE::Rgba8::BLACK;
			PBE::Rgba8 color3 = PBE::Rgba8::BLACK;

			PBE::Vec3 faceNormal, faceTangent, faceBitangent;
			PBE::AABB2 tex = blockItr.GetBlockDef()->m_TextureCoords[(BlockFace)i];

			switch ( (BlockFace)i )
			{
				case BlockFace::TOP:
				{
					p0 = blockPos + PBE::Vec3(0.f, 0.f, 1.f);
					p1 = blockPos + PBE::Vec3(1.f, 0.f, 1.f);
					p2 = blockPos + PBE::Vec3(0.f, 1.f, 1.f);
					p3 = blockPos + PBE::Vec3(1.f, 1.f, 1.f);
					faceNormal = PBE::Vec3::UP;
					faceTangent = PBE::Vec3::RIGHT;
					faceBitangent = PBE::Vec3::FORWARD;

					BlockIterator ci = blockItr.GetUp();
					BlockIterator ri = blockItr.GetUp().GetRight();
					BlockIterator li = blockItr.GetUp().GetLeft();

					BlockIterator tci = blockItr.GetUp().GetForward();
					BlockIterator tri = blockItr.GetUp().GetForward().GetRight();
					BlockIterator tli = blockItr.GetUp().GetForward().GetLeft();

					BlockIterator bci = blockItr.GetUp().GetBack();
					BlockIterator bri = blockItr.GetUp().GetBack().GetRight();
					BlockIterator bli = blockItr.GetUp().GetBack().GetLeft();

					EvaluateColorsFromBlockIterators(ci, ri, li, tci, tri, tli, bci, bri, bli, color0, color1, color2, color3, flip);
					break;
				}
				case BlockFace::BOTTOM:
				{
					p0 = blockPos + PBE::Vec3(0.f, 0.f, 0.f);
					p1 = blockPos + PBE::Vec3(0.f, 1.f, 0.f);
					p2 = blockPos + PBE::Vec3(1.f, 0.f, 0.f);
					p3 = blockPos + PBE::Vec3(1.f, 1.f, 0.f);
					faceNormal = PBE::Vec3::DOWN;
					faceTangent = PBE::Vec3::LEFT;
					faceBitangent = PBE::Vec3::FORWARD;

					BlockIterator ci = blockItr.GetDown();
					BlockIterator ri = blockItr.GetDown().GetRight();
					BlockIterator li = blockItr.GetDown().GetLeft();

					BlockIterator tci = blockItr.GetDown().GetForward();
					BlockIterator tri = blockItr.GetDown().GetForward().GetRight();
					BlockIterator tli = blockItr.GetDown().GetForward().GetLeft();

					BlockIterator bci = blockItr.GetDown().GetBack();
					BlockIterator bri = blockItr.GetDown().GetBack().GetRight();
					BlockIterator bli = blockItr.GetDown().GetBack().GetLeft();

					EvaluateColorsFromBlockIterators(ci, ri, li, tci, tri, tli, bci, bri, bli, color0, color2, color1, color3, flip);
					break;
				}
				case BlockFace::FRONT:
				{
					p0 = blockPos + PBE::Vec3(1.f, 1.f, 0.f);
					p1 = blockPos + PBE::Vec3(1.f, 1.f, 1.f);
					p2 = blockPos + PBE::Vec3(1.f, 0.f, 0.f);
					p3 = blockPos + PBE::Vec3(1.f, 0.f, 1.f);

					faceNormal = PBE::Vec3::FORWARD;
					faceTangent = PBE::Vec3::UP;
					faceBitangent = PBE::Vec3::RIGHT;

					BlockIterator ci = blockItr.GetForward();
					BlockIterator li = blockItr.GetForward().GetRight();
					BlockIterator ri = blockItr.GetForward().GetLeft();

					BlockIterator tci = blockItr.GetForward().GetUp();
					BlockIterator tli = blockItr.GetForward().GetUp().GetRight();
					BlockIterator tri = blockItr.GetForward().GetUp().GetLeft();

					BlockIterator bci = blockItr.GetForward().GetDown();
					BlockIterator bli = blockItr.GetForward().GetDown().GetRight();
					BlockIterator bri = blockItr.GetForward().GetDown().GetLeft();

					EvaluateColorsFromBlockIterators(ci, ri, li, tci, tri, tli, bci, bri, bli, color0, color1, color2, color3, flip);
					break;
				}
				case BlockFace::BACK:
				{
					p0 = blockPos + PBE::Vec3(0.f, 0.f, 0.f);
					p2 = blockPos + PBE::Vec3(0.f, 1.f, 0.f);
					p1 = blockPos + PBE::Vec3(0.f, 0.f, 1.f);
					p3 = blockPos + PBE::Vec3(0.f, 1.f, 1.f);
					faceNormal = PBE::Vec3::BACKWARD;
					faceTangent = PBE::Vec3::UP;
					faceBitangent = PBE::Vec3::LEFT;

					BlockIterator ci = blockItr.GetBack();
					BlockIterator ri = blockItr.GetBack().GetRight();
					BlockIterator li = blockItr.GetBack().GetLeft();

					BlockIterator tci = blockItr.GetBack().GetUp();
					BlockIterator tri = blockItr.GetBack().GetUp().GetRight();
					BlockIterator tli = blockItr.GetBack().GetUp().GetLeft();

					BlockIterator bci = blockItr.GetBack().GetDown();
					BlockIterator bri = blockItr.GetBack().GetDown().GetRight();
					BlockIterator bli = blockItr.GetBack().GetDown().GetLeft();

					EvaluateColorsFromBlockIterators(ci, ri, li, tci, tri, tli, bci, bri, bli, color0, color1, color2, color3, flip);
					break;
				}
				case BlockFace::LEFT:
				{
					p0 = blockPos + PBE::Vec3(0.f, 1.f, 0.f);
					p1 = blockPos + PBE::Vec3(0.f, 1.f, 1.f);
					p2 = blockPos + PBE::Vec3(1.f, 1.f, 0.f);
					p3 = blockPos + PBE::Vec3(1.f, 1.f, 1.f);
					faceNormal = PBE::Vec3::LEFT;
					faceTangent = PBE::Vec3::FORWARD;
					faceBitangent = PBE::Vec3::UP;

					BlockIterator ci = blockItr.GetLeft();
					BlockIterator ri = blockItr.GetLeft().GetBack();
					BlockIterator li = blockItr.GetLeft().GetForward();

					BlockIterator tci = blockItr.GetLeft().GetUp();
					BlockIterator tri = blockItr.GetLeft().GetUp().GetBack();
					BlockIterator tli = blockItr.GetLeft().GetUp().GetForward();

					BlockIterator bci = blockItr.GetLeft().GetDown();
					BlockIterator bri = blockItr.GetLeft().GetDown().GetBack();
					BlockIterator bli = blockItr.GetLeft().GetDown().GetForward();

					EvaluateColorsFromBlockIterators(ci, ri, li, tci, tri, tli, bci, bri, bli, color0, color1, color2, color3, flip);
					break;
				}
				case BlockFace::RIGHT:
				{
					p0 = blockPos + PBE::Vec3(1.f, 0.f, 0.f);
					p1 = blockPos + PBE::Vec3(1.f, 0.f, 1.f);
					p2 = blockPos + PBE::Vec3(0.f, 0.f, 0.f);
					p3 = blockPos + PBE::Vec3(0.f, 0.f, 1.f);

					faceNormal = PBE::Vec3::RIGHT;
					faceTangent = PBE::Vec3::FORWARD;
					faceBitangent = PBE::Vec3::DOWN;

					BlockIterator ci = blockItr.GetRight();
					BlockIterator ri = blockItr.GetRight().GetForward();
					BlockIterator li = blockItr.GetRight().GetBack();

					BlockIterator tci = blockItr.GetRight().GetUp();
					BlockIterator tri = blockItr.GetRight().GetUp().GetForward();
					BlockIterator tli = blockItr.GetRight().GetUp().GetBack();

					BlockIterator bci = blockItr.GetRight().GetDown();
					BlockIterator bri = blockItr.GetRight().GetDown().GetForward();
					BlockIterator bli = blockItr.GetRight().GetDown().GetBack();

					EvaluateColorsFromBlockIterators(ci, ri, li, tci, tri, tli, bci, bri, bli, color0, color1, color2, color3, flip);
					break;
				}
			}
			PBE::Rgba8 color = PBE::Rgba8::BLACK;
			float blockLight = (float)light->GetBlockLight();
			float t = blockLight / 15.f;
			t *= t;
			float ambientRadiance = t * t * (3.f - 2.f * t);
			color.r = (uint8_t)(ambientRadiance * 255.f);

			float skyLight = (float)light->GetSkyLight();
			t = skyLight / 15.f;
			t *= t;
			float skyRadiance = t * t * (3.f - 2.f * t);
			color.g = (uint8_t)(skyRadiance * 255.f);
			color.a = 255;
			AddVertsForBlockFace(p0, p1, p2, p3,color0,color1,color2,color3, faceNormal, faceTangent, faceBitangent, tex, flip, isOpaque);
		}
	}
}

void Chunk::CmdDraw(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	if (m_State == ChunkState::INACTIVE)
	{
		return;
	}

	m_GPUMeshesInFlight[frameIndex] = m_GPUMesh;
	if (m_GPUMesh)
	{
		PBE::CameraModelPushConstantData data;
		data.m_Model = PBE::Mat4::CreateTranslation3D(
			PBE::Vec3((float)m_ChunkCoords.x * CHUNK_X_SIZE, (float)m_ChunkCoords.y * CHUNK_Y_SIZE, (float)m_ChunkCoords.z * CHUNK_Z_SIZE)
		);
		vkCmdPushConstants(cmd, renderer->GetHDRPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PBE::CameraModelPushConstantData), &data);
		m_GPUMesh->CmdBind(cmd);
		m_GPUMesh->CmdDraw(cmd);
	}
}

void Chunk::CmdDrawTranslucent(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	if (m_State == ChunkState::INACTIVE)
	{
		return;
	}

	m_TranslucentGPUMeshesInFlight[frameIndex] = m_TranslucentGPUMesh;
	if (m_TranslucentGPUMesh)
	{
		PBE::CameraModelPushConstantData data;
		data.m_Model = PBE::Mat4::CreateTranslation3D(
			PBE::Vec3((float)m_ChunkCoords.x * CHUNK_X_SIZE, (float)m_ChunkCoords.y * CHUNK_Y_SIZE, (float)m_ChunkCoords.z * CHUNK_Z_SIZE)
		);
		vkCmdPushConstants(cmd, renderer->GetHDRPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PBE::CameraModelPushConstantData), &data);
		m_TranslucentGPUMesh->CmdBind(cmd);
		m_TranslucentGPUMesh->CmdDraw(cmd);
	}
}

void Chunk::TryLoad(ChunkRegion* region)
{
	if (m_BlockWorld->m_NetMode != BlockWorldNetMode::SERVER)
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Header.m_ID = CubikkiMessageType::RequestChunk;
		msg << m_ChunkCoords;

		g_Client->Send(msg);
		m_State = ChunkState::GENERATING_BLOCKS;
		return;
	}
	if (region)
	{
		ChunkCompressedData* data = region->GetChunkCompressedDataPointer(m_ChunkCoords);
		if (data)
		{
			m_BlockStorage.LoadBlockIndices(data->m_CompressedBlockIndices);
			m_BlockStorage.LoadExtendedBlockIndices(data->m_CompressedExtendedBlockIndices);
			m_BlockStorage.LoadBlockEntries(data->m_CompressedBlockData);

			m_HasBlockData = true;
			CreateAndMarkInitialLighting();
			m_State = ChunkState::BUILDING_LIGHTING;
		}
		else
		{
			m_State = ChunkState::GENERATING_BLOCKS;
		}
	}
}

void Chunk::Save()
{
	if (m_BlockWorld->m_NetMode != BlockWorldNetMode::SERVER)
	{
		return;
	}
	if (!m_NeedsSaving)
	{
		m_State = ChunkState::COMPLETE;
		return;
	}
	ChunkCompressedData* data = new ChunkCompressedData();
	data->m_ChunkPos = m_ChunkCoords;
	data->m_CompressedBlockIndices = std::move(m_BlockStorage.GetCompressedBlockIndices());
	data->m_CompressedExtendedBlockIndices = std::move(m_BlockStorage.GetCompressedExtendedBlockIndices());
	data->m_CompressedBlockData = std::move(m_BlockStorage.GetCompressedBlockData());
	PBE::IntVec3 regionCoords = GetRegionCoordsFromGlobalChunkCoords(m_ChunkCoords);
	ChunkRegion* region = m_BlockWorld->GetChunkRegion(regionCoords);
	if (region)
	{
		region->SetChunkCompressedDataPointer(m_ChunkCoords, data);
	}

	m_State = ChunkState::COMPLETE;
}

void Chunk::SendChunkToClient(PBE::NetConnection<CubikkiMessageType>& client)
{
	if (m_HasBlockData)
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Header.m_ID = CubikkiMessageType::ChunkData;

		PBE::GrowBuffer buffer;
		std::vector<uint8_t> compressedBlockIndices = m_BlockStorage.GetCompressedBlockIndices();
		std::vector<uint8_t> compressedExtendedBlockIndices = m_BlockStorage.GetCompressedExtendedBlockIndices();
		std::vector<uint8_t> compressedBlockData = m_BlockStorage.GetCompressedBlockData();

		buffer.WriteDynamicArray(compressedBlockIndices.data(), compressedBlockIndices.size());
		buffer.WriteDynamicArray(compressedExtendedBlockIndices.data(), compressedExtendedBlockIndices.size());
		buffer.WriteDynamicArray(compressedBlockData.data(), compressedBlockData.size());

		msg.m_Body = buffer.GetBuffer();
		msg.m_Header.m_Size = (uint32_t)msg.m_Body.size();
		msg << m_ChunkCoords;

		client.Send(msg);
	}
}

void Chunk::RecieveChunkFromServer(PBE::NetMessage<CubikkiMessageType>& client)
{
	m_ChunkRecieveMutex.lock();
	PBE::BufferReader reader(
		client.m_Body
	);

	std::vector<uint8_t> compressedBlockIndices;
	std::vector<uint8_t> compressedExtendedBlockIndices;
	std::vector<uint8_t> compressedBlockData;

	reader.ReadDynamicArray(compressedBlockIndices);
	reader.ReadDynamicArray(compressedExtendedBlockIndices);
	reader.ReadDynamicArray(compressedBlockData);

	m_BlockStorage.LoadBlockIndices(compressedBlockIndices);
	m_BlockStorage.LoadExtendedBlockIndices(compressedExtendedBlockIndices);
	m_BlockStorage.LoadBlockEntries(compressedBlockData);
	m_ChunkRecieveMutex.unlock();
	CreateAndMarkInitialLighting();
	m_State = ChunkState::BUILDING_LIGHTING;
}

void Chunk::SetBlock(PBE::IntVec3 const& localPos, Block const& block)
{
	m_NeedsSaving = true;

	PBE::IntVec3 chunkGlobalBlockCoords = PBE::IntVec3(m_ChunkCoords.x * CHUNK_X_SIZE, m_ChunkCoords.y * CHUNK_Y_SIZE, m_ChunkCoords.z * CHUNK_Z_SIZE);
	PBE::IntVec3 blockGlobalCoords = chunkGlobalBlockCoords + localPos;

	Block currentBlock = GetBlock(localPos);
	g_BlockBrokenEvents[currentBlock.m_BlockID].Invoke(blockGlobalCoords);
	int index = GetBlockIndexFromLocalBlockCoords(localPos);
	m_BlockStorage.SetBlock(index, block);
	g_BlockPlacedEvents[block.m_BlockID].Invoke(blockGlobalCoords);

	BlockDef const& def = g_BlockDefs[block.m_BlockID];

	BlockIterator itr = BlockIterator(this, index);
	MarkBlockLightDirty(itr);

	for (int i = 0; i < (int)BlockFace::COUNT; ++i)
	{
		BlockIterator neighbor = itr.GetNeighbor((BlockFace)i);
		if (neighbor.IsValid())
		{
			neighbor.m_pChunk->m_MeshDirty = true;
		}
	}
}

Block Chunk::GetBlock(PBE::IntVec3 const& localPos) const
{
	int index = GetBlockIndexFromLocalBlockCoords(localPos);
	return m_BlockStorage.GetBlock(index);
}

#pragma region Neighbor Add / Remove
void Chunk::AddNeighborUp(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkUp = chunk;
	chunk->m_ChunkDown = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborDown(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkDown = chunk;
	chunk->m_ChunkUp = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborForward(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkForward = chunk;
	chunk->m_ChunkBack = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborBack(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkBack = chunk;
	chunk->m_ChunkForward = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborLeft(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkLeft = chunk;
	chunk->m_ChunkRight = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborRight(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkRight = chunk;
	chunk->m_ChunkLeft = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborUpForward(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkUpForward = chunk;
	chunk->m_ChunkDownBack = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborUpBack(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkUpBack = chunk;
	chunk->m_ChunkDownForward = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborUpLeft(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkUpLeft = chunk;
	chunk->m_ChunkDownRight = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborUpRight(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkUpRight = chunk;
	chunk->m_ChunkDownLeft = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborDownForward(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkDownForward = chunk;
	chunk->m_ChunkUpBack = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborDownBack(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkDownBack = chunk;
	chunk->m_ChunkUpForward = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborDownLeft(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkDownLeft = chunk;
	chunk->m_ChunkUpRight = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborDownRight(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkDownRight = chunk;
	chunk->m_ChunkUpLeft = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborForwardLeft(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkForwardLeft = chunk;
	chunk->m_ChunkBackRight = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborForwardRight(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkForwardRight = chunk;
	chunk->m_ChunkBackLeft = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborBackLeft(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkBackLeft = chunk;
	chunk->m_ChunkForwardRight = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::AddNeighborBackRight(Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}
	m_ChunkBackRight = chunk;
	chunk->m_ChunkForwardLeft = this;
	m_MeshDirty = true;
	chunk->m_MeshDirty = true;
}

void Chunk::RemoveNeighborUp()
{
	if (m_ChunkUp)
	{
		m_ChunkUp->m_ChunkDown = nullptr;
		m_ChunkUp = nullptr;
	}
}

void Chunk::RemoveNeighborDown()
{
	if (m_ChunkDown)
	{
		m_ChunkDown->m_ChunkUp = nullptr;
		m_ChunkDown = nullptr;
	}
}

void Chunk::RemoveNeighborForward()
{
	if (m_ChunkForward)
	{
		m_ChunkForward->m_ChunkBack = nullptr;
		m_ChunkForward = nullptr;
	}
}

void Chunk::RemoveNeighborBack()
{
	if (m_ChunkBack)
	{
		m_ChunkBack->m_ChunkForward = nullptr;
		m_ChunkBack = nullptr;
	}
}

void Chunk::RemoveNeighborLeft()
{
	if (m_ChunkLeft)
	{
		m_ChunkLeft->m_ChunkRight = nullptr;
		m_ChunkLeft = nullptr;
	}
}

void Chunk::RemoveNeighborRight()
{
	if (m_ChunkRight)
	{
		m_ChunkRight->m_ChunkLeft = nullptr;
		m_ChunkRight = nullptr;
	}
}

void Chunk::RemoveNeighborUpForward()
{
	if (m_ChunkUpForward)
	{
		m_ChunkUpForward->m_ChunkDownBack = nullptr;
		m_ChunkUpForward = nullptr;
	}
}

void Chunk::RemoveNeighborUpBack()
{
	if (m_ChunkUpBack)
	{
		m_ChunkUpBack->m_ChunkDownForward = nullptr;
		m_ChunkUpBack = nullptr;
	}
}

void Chunk::RemoveNeighborUpLeft()
{
	if (m_ChunkUpLeft)
	{
		m_ChunkUpLeft->m_ChunkDownRight = nullptr;
		m_ChunkUpLeft = nullptr;
	}
}

void Chunk::RemoveNeighborUpRight()
{
	if (m_ChunkUpRight)
	{
		m_ChunkUpRight->m_ChunkDownLeft = nullptr;
		m_ChunkUpRight = nullptr;
	}
}

void Chunk::RemoveNeighborDownForward()
{
	if (m_ChunkDownForward)
	{
		m_ChunkDownForward->m_ChunkUpBack = nullptr;
		m_ChunkDownForward = nullptr;
	}
}

void Chunk::RemoveNeighborDownBack()
{
	if (m_ChunkDownBack)
	{
		m_ChunkDownBack->m_ChunkUpForward = nullptr;
		m_ChunkDownBack = nullptr;
	}
}

void Chunk::RemoveNeighborDownLeft()
{
	if (m_ChunkDownLeft)
	{
		m_ChunkDownLeft->m_ChunkUpRight = nullptr;
		m_ChunkDownLeft = nullptr;
	}
}

void Chunk::RemoveNeighborDownRight()
{
	if (m_ChunkDownRight)
	{
		m_ChunkDownRight->m_ChunkUpLeft = nullptr;
		m_ChunkDownRight = nullptr;
	}
}

void Chunk::RemoveNeighborForwardLeft()
{
	if (m_ChunkForwardLeft)
	{
		m_ChunkForwardLeft->m_ChunkBackRight = nullptr;
		m_ChunkForwardLeft = nullptr;
	}
}

void Chunk::RemoveNeighborForwardRight()
{
	if (m_ChunkForwardRight)
	{
		m_ChunkForwardRight->m_ChunkBackLeft = nullptr;
		m_ChunkForwardRight = nullptr;
	}
}

void Chunk::RemoveNeighborBackLeft()
{
	if (m_ChunkBackLeft)
	{
		m_ChunkBackLeft->m_ChunkForwardRight = nullptr;
		m_ChunkBackLeft = nullptr;
	}
}

void Chunk::RemoveNeighborBackRight()
{
	if (m_ChunkBackRight)
	{
		m_ChunkBackRight->m_ChunkForwardLeft = nullptr;
		m_ChunkBackRight = nullptr;
	}
}

std::array<Chunk*, 26> Chunk::GetBoxingNeighbors() const
{
	std::array<Chunk*, 26> neighbors;
	
	neighbors[0] = m_ChunkUp;
	neighbors[1] = m_ChunkDown;
	neighbors[2] = m_ChunkForward;
	neighbors[3] = m_ChunkBack;
	neighbors[4] = m_ChunkLeft;
	neighbors[5] = m_ChunkRight;

	neighbors[6] = m_ChunkUpForward;
	neighbors[7] = m_ChunkUpBack;
	neighbors[8] = m_ChunkUpLeft;
	neighbors[9] = m_ChunkUpRight;

	neighbors[10] = m_ChunkDownForward;
	neighbors[11] = m_ChunkDownBack;
	neighbors[12] = m_ChunkDownLeft;
	neighbors[13] = m_ChunkDownRight;

	neighbors[14] = m_ChunkForwardRight;
	neighbors[15] = m_ChunkForwardLeft;
	neighbors[16] = m_ChunkBackRight;
	neighbors[17] = m_ChunkBackLeft;

	neighbors[18] = neighbors[6] ? neighbors[6]->m_ChunkRight : nullptr;
	neighbors[19] = neighbors[6] ? neighbors[6]->m_ChunkLeft : nullptr;
	neighbors[20] = neighbors[7] ? neighbors[7]->m_ChunkRight : nullptr;
	neighbors[21] = neighbors[7] ? neighbors[7]->m_ChunkLeft : nullptr;

	neighbors[22] = neighbors[10] ? neighbors[10]->m_ChunkRight : nullptr;
	neighbors[23] = neighbors[10] ? neighbors[10]->m_ChunkLeft : nullptr;
	neighbors[24] = neighbors[11] ? neighbors[11]->m_ChunkRight : nullptr;
	neighbors[25] = neighbors[11] ? neighbors[11]->m_ChunkLeft : nullptr;

	return neighbors;
}
#pragma endregion

bool Chunk::AllBoxingNeighborsHaveBlockData() const
{
	if (!m_ChunkLeft)
	{
		return false;
	}
	if (!m_ChunkRight)
	{
		return false;
	}
	if (!m_ChunkUp)
	{
		return false;
	}
	if (!m_ChunkDown)
	{
		return false;
	}
	if (!m_ChunkForward)
	{
		return false;
	}
	if (!m_ChunkBack)
	{
		return false;
	}
	if (!m_ChunkUpLeft)
	{
		return false;
	}
	if (!m_ChunkUpRight)
	{
		return false;
	}
	if (!m_ChunkUpForward)
	{
		return false;
	}
	if (!m_ChunkUpBack)
	{
		return false;
	}
	if (!m_ChunkDownLeft)
	{
		return false;
	}
	if (!m_ChunkDownRight)
	{
		return false;
	}
	if (!m_ChunkDownForward)
	{
		return false;
	}
	if (!m_ChunkDownBack)
	{
		return false;
	}
	if (!m_ChunkForwardLeft)
	{
		return false;
	}
	if (!m_ChunkForwardRight)
	{
		return false;
	}
	if (!m_ChunkBackLeft)
	{
		return false;
	}
	if (!m_ChunkBackRight)
	{
		return false;
	}
	if (m_ChunkLeft->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkRight->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkUp->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkDown->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkForward->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkBack->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkUpLeft->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkUpRight->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkUpForward->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkUpBack->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkDownLeft->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkDownRight->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkDownForward->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkDownBack->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkForwardLeft->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkForwardRight->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkBackLeft->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkBackRight->m_State == ChunkState::GENERATING_BLOCKS)
	{
		return false;
	}
	if (m_ChunkLeft->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkRight->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkUp->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkDown->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkForward->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkBack->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkUpLeft->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkUpRight->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkUpForward->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkUpBack->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkDownLeft->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkDownRight->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkDownForward->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkDownBack->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkForwardLeft->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkForwardRight->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkBackLeft->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkBackRight->m_State == ChunkState::LOADING)
	{
		return false;
	}
	if (m_ChunkLeft->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkRight->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkUp->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkDown->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkForward->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkBack->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkUpLeft->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkUpRight->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkUpForward->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkUpBack->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkDownLeft->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkDownRight->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkDownForward->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkDownBack->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkForwardLeft->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkForwardRight->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkBackLeft->m_State == ChunkState::INACTIVE)
	{
		return false;
	}
	if (m_ChunkBackRight->m_State == ChunkState::INACTIVE)
	{
		return false;
	}

	return true;
}

bool Chunk::AllBoxingNeighborsHaveLightData() const
{
	std::array<Chunk*, 26> neighbors = GetBoxingNeighbors();
	for (Chunk* neighbor : neighbors)
	{
		if (!neighbor)
		{
			return false;
		}
		if (
			neighbor->m_State == ChunkState::GENERATING_BLOCKS ||
			neighbor->m_State == ChunkState::LOADING ||
			neighbor->m_State == ChunkState::INACTIVE ||
			neighbor->m_State == ChunkState::BUILDING_LIGHTING
			)
		{
			return false;
		}
	}
	return true;
}

void ChunkTryLoadJob::Execute()
{
	PBE::IntVec3 regionCoords = GetRegionCoordsFromGlobalChunkCoords(m_Chunk->m_ChunkCoords);
	ChunkRegion* region = m_Chunk->m_BlockWorld->GetChunkRegion(regionCoords);
	if (region)
	{
		m_Chunk->TryLoad(region);
	}
}