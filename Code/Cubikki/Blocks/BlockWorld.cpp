#include "Cubikki/Actors/ActorUtils.hpp"

#include "Cubikki/Blocks/BlockWorld.hpp"
#include "Cubikki/Blocks/WorldGen/FlatWorldGenerator.hpp"
#include "Cubikki/Blocks/Physics/BlockPhysicsWorld.hpp"

#include "Cubikki/Common.hpp"

#include "Engine/Graphics/GraphicsUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/BufferReader.hpp"
#include "Engine/Core/GrowBuffer.hpp"

#include "imgui.h"

#include <iostream>
BlockWorld* g_BlockWorld;
BlockWorld::BlockWorld(std::string_view fileName, CubikkiServer* server)
	: BlockWorld(fileName)
{
	m_NetMode = BlockWorldNetMode::SERVER;
	m_Server = server;
}

BlockWorld::BlockWorld(std::string_view fileName, CubikkiClient* client)
	: BlockWorld(fileName)
{
	m_NetMode = BlockWorldNetMode::CLIENT;
	m_Client = client;
}

BlockWorld::BlockWorld(std::string_view fileName)
	: m_DebugTimer(nullptr, 1.0f)
	, m_UpdateTimer(nullptr, 1.0f / 24.f)
	, m_RandomTickTimer(nullptr, 1.0f / 20.f)
{
	g_BlockWorld = this;
	BlockPhysicsWorldCreateInfo pwInfo;
	pwInfo.m_pWorld = this;
	pwInfo.m_Gravity = PBE::Vec3(0.f, 0.f, -20.f);
	m_PhysicsWorld = new BlockPhysicsWorld(pwInfo);
	LoadBlockDefs();

	m_FileJobSystem = new PBE::JobSystem(1);

	m_WorldGenerators[WORLD_GEN_FLAT] = new FlatWorldGenerator();
	m_WorldGenerator = m_WorldGenerators[WORLD_GEN_FLAT];


	PBE::MaterialCreateInfo materialInfo = {};
	materialInfo.m_Allocator = g_Engine->m_pRenderer->m_Allocator;
	materialInfo.m_Device = g_Engine->m_pRenderer->m_Device;
	materialInfo.m_DiffuseTexture = g_BlockAtlas.lock()->GetTexture();
	materialInfo.m_MaterialConstants = {};
	materialInfo.m_NormalTexture = g_Engine->m_pRenderer->GetWhiteTexture();
	materialInfo.m_ParallaxTexture = g_Engine->m_pRenderer->GetWhiteTexture();
	materialInfo.m_SpecularGlossEmitTexture = g_Engine->m_pRenderer->GetWhiteTexture();

	m_Material = g_Engine->m_pRenderer->CreateMaterial(materialInfo);
	
	g_Engine->m_pLuaSystem->ReloadAllScripts();

	m_ActorSystem = new ActorSystem();
	m_ActorSystem->Startup();

	PBE::ShaderModuleLoadInfo shaderInfo;
	shaderInfo.m_Path = "Assets/Shaders/ProgrammableSkybox.vert";
	shaderInfo.m_Stage = PBE::ShaderStage::VERTEX;
	PBE::ShaderModuleLoadInfo shaderInfo2;
	shaderInfo2.m_Path = "Assets/Shaders/ProgrammableSkybox.frag";
	shaderInfo2.m_Stage = PBE::ShaderStage::FRAGMENT;

	auto builder = g_Engine->m_pRenderer->CreateHDRPipelineBuilder();
	builder.m_ShaderStages[0].flags = VK_SHADER_STAGE_VERTEX_BIT;
	builder.m_ShaderStages[0].module = g_Engine->m_pRenderer->CreateOrGetShaderModule(shaderInfo);
	builder.m_ShaderStages[0].pName = "main";
	builder.m_ShaderStages[1].flags = VK_SHADER_STAGE_FRAGMENT_BIT;
	builder.m_ShaderStages[1].module = g_Engine->m_pRenderer->CreateOrGetShaderModule(shaderInfo2);
	builder.m_ShaderStages[1].pName = "main";

	builder.m_DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	builder.m_DepthStencilInfo.depthWriteEnable = VK_TRUE;
	m_SkyboxPipeline = builder.CreatePipeline(g_Engine->m_pRenderer->m_Device, "Skybox Pipeline");
	
	int indices[6] { 0, 1, 2, 2, 3, 0 };
	m_QuadBuffer = (PBE::GPUBuffer*)malloc(sizeof(PBE::GPUBuffer));
	*m_QuadBuffer = PBE::CreateGPUBuffer(g_Engine->m_pRenderer->m_Allocator, sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU );
	m_QuadBuffer->Map(g_Engine->m_pRenderer->m_Allocator);
	memcpy(m_QuadBuffer->m_Data, indices, sizeof(indices));
	m_QuadBuffer->Unmap(g_Engine->m_pRenderer->m_Allocator);
	vmaSetAllocationName(g_Engine->m_pRenderer->m_Allocator, m_QuadBuffer->m_Allocation, "Quad Buffer");
}


BlockWorld::~BlockWorld()
{
	g_Engine->m_pRenderer->WaitIdle();
	vkDestroyPipeline(g_Engine->m_pRenderer->m_Device, m_SkyboxPipeline, nullptr);

	m_Chunks.UpdateChunksVector();
	std::vector<ChunkSaveJob*> saveJobs;
	for (Chunk* chunk : m_Chunks.m_Chunks)
	{ 
		if (chunk)
		{
			if (chunk->m_State == ChunkState::COMPLETE)
			{
				ChunkSaveJob* job = new ChunkSaveJob(chunk);
				saveJobs.push_back(job);
				g_Engine->m_pJobSystem->SubmitJob(job);
			}
		}
	}

	for (ChunkSaveJob* job : saveJobs)
	{
		g_Engine->m_pJobSystem->WaitUntilAndClaimJobIfComplete(job);
		delete job;
	}

	bool saving = true;
	while (saving)
	{
		saving = false;
		for (Chunk* chunk : m_Chunks.m_Chunks)
		{
			if (chunk)
			{
				chunk->Update();
				if (chunk->m_State == ChunkState::SAVING)
				{
					saving = true;
					break;
				}
			}
		}
	}

	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			chunk->WaitForJobsToComplete();
		}
	}

	for (auto regionItr : m_ChunkRegions)
	{
		delete regionItr.second;
	}
	m_ChunkRegions.clear();

	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			delete chunk;
		}
	}

	for (Chunk* chunk : m_ChunkPool)
	{
		delete chunk;
	}

	for (WorldGenerator* generator : m_WorldGenerators)
	{
		delete generator;
	}

	delete m_Material;
	delete m_FileJobSystem;
	delete m_ActorSystem;
	PBE::DestroyGPUBuffer(g_Engine->m_pRenderer->m_Allocator, *m_QuadBuffer);
}

void BlockWorld::LoadBlockDefs()
{
	BlockDef def;
	def.m_IsVisible = false;
	def.m_IsSolid = false;
	def.m_IsOpaque = false;
	def.m_BlockLightIntensity = 0;
	RegisterBlockDef("Air", def);
	def.m_IsVisible = true;
	def.m_IsSolid = true;
	def.m_IsOpaque = true;
	def.m_TextureCoords[BlockFace::TOP] = g_BlockAtlas.lock()->GetTextureRegion("GrassTop.png").m_UV;
	def.m_TextureCoords[BlockFace::RIGHT] = g_BlockAtlas.lock()->GetTextureRegion("GrassSide.png").m_UV;
	def.m_TextureCoords[BlockFace::LEFT] = g_BlockAtlas.lock()->GetTextureRegion("GrassSide.png").m_UV;
	def.m_TextureCoords[BlockFace::FRONT] = g_BlockAtlas.lock()->GetTextureRegion("GrassSide.png").m_UV;
	def.m_TextureCoords[BlockFace::BACK] = g_BlockAtlas.lock()->GetTextureRegion("GrassSide.png").m_UV;
	def.m_TextureCoords[BlockFace::BOTTOM] = g_BlockAtlas.lock()->GetTextureRegion("Earth.png").m_UV;
	g_Engine->m_pAudioSystem->CreateSound("Assets/Audio/Blocks/grass_block_place.mp3",PBE::AUDIO_MODE_3D, &def.m_PlaceSound);
	g_Engine->m_pAudioSystem->CreateSound("Assets/Audio/Blocks/grass_block_break.mp3",PBE::AUDIO_MODE_3D, &def.m_BreakSound);

	RegisterBlockDef("Grass", def);

	for (int i = 0; i < BlockFace::COUNT; ++i)
	{
		def.m_TextureCoords[i] = g_BlockAtlas.lock()->GetTextureRegion("Earth.png").m_UV;
	}
	def.m_BlockLightIntensity = 8;

	g_Engine->m_pAudioSystem->CreateSound("Assets/Audio/Blocks/earth_block_place.mp3",PBE::AUDIO_MODE_3D, &def.m_PlaceSound);
	g_Engine->m_pAudioSystem->CreateSound("Assets/Audio/Blocks/earth_block_break.mp3",PBE::AUDIO_MODE_3D, &def.m_BreakSound);

	RegisterBlockDef("Earth", def);

	for (int i = 0; i < BlockFace::COUNT; ++i)
	{
		def.m_TextureCoords[i] = g_BlockAtlas.lock()->GetTextureRegion("Stone2.png").m_UV;
	}
	def.m_BlockLightIntensity = 0;

	g_Engine->m_pAudioSystem->CreateSound("Assets/Audio/Blocks/stone_block_place.mp3",PBE::AUDIO_MODE_3D, &def.m_PlaceSound);
	g_Engine->m_pAudioSystem->CreateSound("Assets/Audio/Blocks/stone_block_break.mp3",PBE::AUDIO_MODE_3D, &def.m_BreakSound);
	RegisterBlockDef("Stone2", def);

	for (int i = 0; i < BlockFace::COUNT; ++i)
	{
		def.m_TextureCoords[i] = g_BlockAtlas.lock()->GetTextureRegion("Water.png").m_UV;
	}

	def.m_IsSolid = false;
	def.m_IsOpaque = false;
	def.m_BlockLightIntensity = 0;

	RegisterBlockDef("Water", def);

	for (int i = 0; i < BlockFace::COUNT; ++i)
	{
		def.m_TextureCoords[i] = g_BlockAtlas.lock()->GetTextureRegion("Sand.png").m_UV;
	}

	def.m_IsSolid = true;
	def.m_IsOpaque = true;
	def.m_BlockLightIntensity = 0;

	RegisterBlockDef("Sand", def);

	for (int i = 0; i < BlockFace::COUNT; ++i)
	{
		def.m_TextureCoords[i] = g_BlockAtlas.lock()->GetTextureRegion("Plank.png").m_UV;
	}

	RegisterBlockDef("Planks", def);

	for (int i = 0; i < BlockFace::COUNT; ++i)
	{
		def.m_TextureCoords[i] = g_BlockAtlas.lock()->GetTextureRegion("Brick.png").m_UV;
	}

	RegisterBlockDef("Bricks", def);

}

void BlockWorld::AddPlayer(std::string_view playerName, PBE::IntVec3 position)
{
	m_PlayerPositions[playerName.data()] = position;
}

void BlockWorld::SetPlayerPosition(std::string_view playerName, PBE::IntVec3 position)
{
	m_PlayerPositions[playerName.data()] = position;

	if (m_NetMode == BlockWorldNetMode::SERVER)
	{
		return;
	}
	PBE::GrowBuffer buffer;
	buffer.WriteString(playerName);
	buffer.Write(position);

	PBE::NetMessage<CubikkiMessageType> msg;
	msg.m_Header.m_ID = CubikkiMessageType::SetPlayerPosition;
	msg.m_Body = buffer.GetBuffer();
	msg.m_Header.m_Size = (uint32_t)msg.m_Body.size();

	m_Client->Send(msg);
}

void BlockWorld::RemovePlayer(std::string_view playerName)
{
	m_PlayerPositions.erase(playerName.data());
}

int BlockWorld::GetSquaredChunkDistanceToNearestPlayer(PBE::IntVec3 position) const
{
	int minDistance = INT_MAX;

	if (m_NetMode != BlockWorldNetMode::SERVER)
	{
		return (GetChunkCoordsFromBlockCoords(m_PlayerPositions.at(m_LocalPlayerName)) - position).GetLengthSquared();
	}

	for (auto& player : m_PlayerPositions)
	{
		int distance = (GetChunkCoordsFromBlockCoords(player.second) - position).GetLengthSquared();
		if (distance < minDistance)
		{
			minDistance = distance;
		}
	}
	return minDistance;
}

int BlockWorld::GetSquaredDistanceToNearestPlayer(PBE::IntVec3 position) const
{
	int minDistance = INT_MAX;
	for (auto& player : m_PlayerPositions)
	{
		int distance = (player.second - position).GetLengthSquared();
		if (distance < minDistance)
		{
			minDistance = distance;
		}
	}
	return minDistance;
}

void BlockWorld::Update()
{
	m_ActorSystem->BeginFrame();
	m_ActorSystem->Update();
	m_ActorSystem->EndFrame();

	m_PhysicsWorld->FixedUpdate(PBE::Clock::GetSystemClock()->GetDeltaSeconds());
	DebugPrintChunks();
	if (!m_UpdateTimer.DecrementPeriodIfElapsed())
	{
		return;
	}

	bool regionLoaded = false;
	for (auto itr = m_LoadChunkRegionJobsAwaitingClaim.begin(); itr != m_LoadChunkRegionJobsAwaitingClaim.end();)
	{
		auto jobPair = *itr;
		LoadChunkRegionJob* job = jobPair.second;
		if (m_FileJobSystem->ClaimJobIfComplete(job))
		{
			itr = m_LoadChunkRegionJobsAwaitingClaim.erase(itr);
			m_ChunkRegions[job->m_RegionPos] = job->m_Region;
			delete job;
			regionLoaded = true;
		}
		else
		{
			++itr;
		}
	}

	float heightViewDistanceFactor = 0.5f;
	int heightViewDistance = (int)(m_ViewDistance * heightViewDistanceFactor);

	struct ChunkRegionSortEntry
	{
		PBE::IntVec3 m_RegionPos;
		int m_Distance;
	};

	std::vector<ChunkRegionSortEntry> regionChunkPositions;
	
	if(m_NetMode == BlockWorldNetMode::SERVER)
	{
		for (auto& player : m_PlayerPositions)
		{
			PBE::IntVec3 playerPos = player.second;
			PBE::IntVec3 playerChunkPos = GetChunkCoordsFromBlockCoords(playerPos);
			//Region Loading
			for (int x = -m_ViewDistance; x <= m_ViewDistance; x++)
			{
				for (int y = -m_ViewDistance; y <= m_ViewDistance; y++)
				{
					for (int z = -heightViewDistance; z <= heightViewDistance; z++)
					{
						PBE::IntVec3 chunkPos = playerChunkPos + PBE::IntVec3(x, y, z);
						PBE::IntVec3 regionPos = GetRegionCoordsFromGlobalChunkCoords(chunkPos);
						if (!IsChunkRegionLoadingOrLoaded(regionPos))
						{
							ChunkRegionSortEntry entry;
							entry.m_RegionPos = regionPos;
							entry.m_Distance = (chunkPos - playerPos).GetLengthSquared();

							regionChunkPositions.push_back(entry);
						}
					}
				}
			}
		}
	}
	else
	{
		PBE::IntVec3 playerPos = m_PlayerPositions[m_LocalPlayerName];
		PBE::IntVec3 playerChunkPos = GetChunkCoordsFromBlockCoords(playerPos);
		//Region Loading
		for (int x = -m_ViewDistance; x <= m_ViewDistance; x++)
		{
			for (int y = -m_ViewDistance; y <= m_ViewDistance; y++)
			{
				for (int z = -heightViewDistance; z <= heightViewDistance; z++)
				{
					PBE::IntVec3 chunkPos = playerChunkPos + PBE::IntVec3(x, y, z);
					PBE::IntVec3 regionPos = GetRegionCoordsFromGlobalChunkCoords(chunkPos);
					if (!IsChunkRegionLoadingOrLoaded(regionPos))
					{
						ChunkRegionSortEntry entry;
						entry.m_RegionPos = regionPos;
						entry.m_Distance = (chunkPos - playerPos).GetLengthSquared();
						regionChunkPositions.push_back(entry);
					}
				}
			}
		}
	}

	std::sort(regionChunkPositions.begin(), regionChunkPositions.end(), [this](ChunkRegionSortEntry const& a, ChunkRegionSortEntry const& b)
		{
			return a.m_Distance > b.m_Distance;
		}
	);

	for (auto& entry : regionChunkPositions)
	{
		PBE::IntVec3 regionPos = entry.m_RegionPos;
		LoadChunkRegionJob* job = new LoadChunkRegionJob(this, regionPos);
		m_LoadChunkRegionJobsAwaitingClaim[regionPos] = job;
		m_FileJobSystem->SubmitJob(job);
	}
	if (m_NetMode == BlockWorldNetMode::SERVER)
	{
		for (auto& player : m_PlayerPositions)
		{
			PBE::IntVec3 playerPos = player.second;
			PBE::IntVec3 playerChunkPos = GetChunkCoordsFromBlockCoords(playerPos);

			for (int x = -m_ViewDistance; x <= m_ViewDistance; ++x)
			{
				for (int y = -m_ViewDistance; y <= m_ViewDistance; ++y)
				{
					for (int z = -heightViewDistance; z <= heightViewDistance; ++z)
					{
						PBE::IntVec3 chunkCoords = playerChunkPos + PBE::IntVec3(x, y, z);
						PBE::IntVec3 regionCoords = GetRegionCoordsFromGlobalChunkCoords(chunkCoords);
						Chunk* chunk = m_Chunks.GetChunk(chunkCoords);
						if (!chunk && m_ChunkRegions.find(regionCoords) != m_ChunkRegions.end())
						{
							Chunk* chunk = GetChunkFromPool(chunkCoords);
							m_NumChunksInRegion[regionCoords]++;
							m_Chunks.SetChunk(chunkCoords, chunk);
						}
					}
				}
			}
		}
	}
	else
	{
		PBE::IntVec3 playerPos = m_PlayerPositions[m_LocalPlayerName];
		PBE::IntVec3 playerChunkPos = GetChunkCoordsFromBlockCoords(playerPos);

		for (int x = -m_ViewDistance; x <= m_ViewDistance; ++x)
		{
			for (int y = -m_ViewDistance; y <= m_ViewDistance; ++y)
			{
				for (int z = -heightViewDistance; z <= heightViewDistance; ++z)
				{
					PBE::IntVec3 chunkCoords = playerChunkPos + PBE::IntVec3(x, y, z);
					PBE::IntVec3 regionCoords = GetRegionCoordsFromGlobalChunkCoords(chunkCoords);
					Chunk* chunk = m_Chunks.GetChunk(chunkCoords);
					if (!chunk && m_ChunkRegions.find(regionCoords) != m_ChunkRegions.end())
					{
						Chunk* chunk = GetChunkFromPool(chunkCoords);
						m_NumChunksInRegion[regionCoords]++;
						m_Chunks.SetChunk(chunkCoords, chunk);
					}
				}
			}
		}
	}

	m_Chunks.UpdateChunksVector();

	std::sort(m_Chunks.m_Chunks.begin(), m_Chunks.m_Chunks.end(), [this](Chunk* a, Chunk* b)
		{
			if (!a)
			{
				return false;
			}
			if (!b)
			{
				return false;
			}

			int aDist = GetSquaredChunkDistanceToNearestPlayer(a->m_ChunkCoords);
			int bDist = GetSquaredChunkDistanceToNearestPlayer(b->m_ChunkCoords);

			return aDist < bDist;
		}
	);
	

	int numChunks = 0;
	int numChunksDeactivated = 0;
	int maxChunks = (int)(2.f * m_ViewDistance * 2.f * m_ViewDistance * 2.f * heightViewDistance * 2.f);
	for (auto itr = m_Chunks.m_Chunks.begin(); itr != m_Chunks.m_Chunks.end(); ++itr)
	{
		if (numChunksDeactivated > m_NumToDeactivatePerFrame)
		{
			break;
		}
		Chunk* chunk = *itr;
		if (chunk)
		{
			++numChunks;
			if (numChunks < maxChunks)
			{
				continue;
			}
			else if (!chunk->m_Active && chunk->m_State == ChunkState::INACTIVE)
			{
				PBE::IntVec3 regionCoords = GetRegionCoordsFromGlobalChunkCoords(chunk->m_ChunkCoords);
				m_NumChunksInRegion[regionCoords]--;
				m_Chunks.SetChunk(chunk->m_ChunkCoords, nullptr);
				chunk->Deactivate();
				ReturnChunkToPool(chunk);
				++numChunksDeactivated;

				if (m_NumChunksInRegion[regionCoords] == 0)
				{
					ChunkRegion* region = m_ChunkRegions[regionCoords];
					m_ChunkRegions.erase(regionCoords);
					delete region;
				}
			}
		}
	}

	m_Chunks.UpdateChunksVector();

	int numActivated = 0;

	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			int dist = GetSquaredChunkDistanceToNearestPlayer(chunk->m_ChunkCoords);
			if (dist > m_ViewDistance * m_ViewDistance)
			{
				if (chunk->m_Active)
				{
					chunk->Deactivate();
				}
			}
			else if (!chunk->m_Active)
			{
				chunk->Activate();

				chunk->AddNeighborBack(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(-1, 0, 0)));
				chunk->AddNeighborForward(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(1, 0, 0)));
				chunk->AddNeighborLeft(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, 1, 0)));
				chunk->AddNeighborRight(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, -1, 0)));
				chunk->AddNeighborUp(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, 0, 1)));
				chunk->AddNeighborDown(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, 0, -1)));
				chunk->AddNeighborUpForward(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(1, 0, 1)));
				chunk->AddNeighborUpBack(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(-1, 0, 1)));
				chunk->AddNeighborUpLeft(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, 1, 1)));
				chunk->AddNeighborUpRight(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, -1, 1)));
				chunk->AddNeighborDownForward(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(1, 0, -1)));
				chunk->AddNeighborDownBack(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(-1, 0, -1)));
				chunk->AddNeighborDownLeft(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, 1, -1)));
				chunk->AddNeighborDownRight(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(0, -1, -1)));
				chunk->AddNeighborForwardLeft(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(1, 1, 0)));
				chunk->AddNeighborForwardRight(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(1, -1, 0)));
				chunk->AddNeighborBackLeft(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(-1, 1, 0)));
				chunk->AddNeighborBackRight(m_Chunks.GetChunk(chunk->m_ChunkCoords + PBE::IntVec3(-1, -1, 0)));

				++numActivated;
				if (numActivated > m_NumToActivatePerFrame)
				{
					break;
				}
			}
		}
	}

	ProcessDirtyLighting();

	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			chunk->Update();
		}
	}

	m_ChunkClientSendQueueMutex.lock();
	for (auto itr = m_ChunkClientSendQueue.begin(); itr != m_ChunkClientSendQueue.end();)
	{
		ChunkClientSendPair& pair = *itr;
		Chunk* chunk = m_Chunks.GetChunk(pair.m_ChunkWorldCoords);
		if (chunk)
		{
			if (!chunk->m_HasBlockData)
			{
				++itr;
			}
			else
			{
				chunk->SendChunkToClient(*pair.m_Client);
				itr = m_ChunkClientSendQueue.erase(itr);
			}
		}
		else
		{
			++itr;
		}
	}
	m_ChunkClientSendQueueMutex.unlock();
}

void BlockWorld::DebugPrintChunks()
{
	if (!g_ShowDebugInfo)
	{
		return;
	}
	int numChunksLighting = 0;
	int numChunksGeneratingBlocks = 0;
	int numChunksGeneratingMesh = 0;
	int numChunksGeneratingGPUMesh = 0;
	int numChunksComplete = 0;
	int numChunksInactive = 0;
	int numChunksSaving = 0;
	int numChunksLoading = 0;

	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			switch (chunk->m_State)
			{
				case ChunkState::INACTIVE: ++numChunksInactive; break;
				case ChunkState::LOADING: ++numChunksLoading; break;
				case ChunkState::BUILDING_LIGHTING: ++numChunksLighting; break;
				case ChunkState::GENERATING_BLOCKS: ++numChunksGeneratingBlocks; break;
				case ChunkState::GENERATING_CPUMESH: ++numChunksGeneratingMesh; break;
				case ChunkState::GENERATING_GPUMESH: ++numChunksGeneratingGPUMesh; break;
				case ChunkState::SAVING: ++numChunksSaving; break;
				case ChunkState::COMPLETE: ++numChunksComplete; break;
			}
		}
	}

	ImGui::Begin("Chunk Debug");
	ImGui::Text("Chunks: %i", m_Chunks.m_NumChunks);
	ImGui::Text("Regions: %i", m_ChunkRegions.size());
	ImGui::Text("Lighting: %i", numChunksLighting);
	ImGui::Text("Generating Blocks: %i", numChunksGeneratingBlocks);
	ImGui::Text("Generating Mesh: %i", numChunksGeneratingMesh);
	ImGui::Text("Generating GPUMesh: %i", numChunksGeneratingGPUMesh);
	ImGui::Text("Complete: %i", numChunksComplete);
	ImGui::Text("Inactive: %i", numChunksInactive);
	ImGui::Text("Saving: %i", numChunksSaving);
	ImGui::Text("Loading: %i", numChunksLoading);
	ImGui::End();
}

void BlockWorld::CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			chunk->CmdTransfer(renderer, cmd, frameIndex);
		}
	}
}

void BlockWorld::CmdDraw(PBE::Camera* camera, PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	float color[4];
	color[0] = 0.1f;
	color[1] = 0.1f;
	color[2] = 0.1f;
	color[3] = 1.0f;

	PBE::VkBeginDebugUtilsLabel(renderer->m_Device, cmd, "Block World Opaque Pass", color);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Engine->m_pRenderer->m_StaticMeshPipeline);
	camera->CmdBind(cmd,g_Engine->m_pRenderer->GetHDRPipelineLayout(), frameIndex);
	m_Material->CmdBind(cmd, g_Engine->m_pRenderer->GetHDRPipelineLayout(), frameIndex);
	PBE::Frustum frustum = camera->GetFrustum();
	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			if (chunk->m_GPUMesh)
			{
				if (frustum.IsAABB3Inside(chunk->m_BoundingBox))
				{
					chunk->CmdDraw(renderer, cmd, frameIndex);
				}
			}
		}
	}
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SkyboxPipeline);
	camera->CmdBind(cmd, g_Engine->m_pRenderer->GetHDRPipelineLayout(), frameIndex);
	m_Material->CmdBind(cmd, g_Engine->m_pRenderer->GetHDRPipelineLayout(), frameIndex);
	vkCmdBindIndexBuffer(cmd, m_QuadBuffer->m_Buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
	PBE::VkEndDebugUtilsLabel(g_Engine->m_pRenderer->m_Device, cmd);
}

void BlockWorld::CmdDrawTranslucent(PBE::Camera* camera, PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex)
{
	float color[4];
	color[0] = 0.1f;
	color[1] = 0.1f;
	color[2] = 0.1f;
	color[3] = 1.0f;

	PBE::VkBeginDebugUtilsLabel(renderer->m_Device, cmd, "Block World Translucent Pass", color);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Engine->m_pRenderer->m_TranslucentStaticMeshPipeline);
	camera->CmdBind(cmd, g_Engine->m_pRenderer->GetHDRPipelineLayout(), frameIndex);
	m_Material->CmdBind(cmd, g_Engine->m_pRenderer->GetHDRPipelineLayout(), frameIndex);
	PBE::Frustum frustum = camera->GetFrustum();
	for (Chunk* chunk : m_Chunks.m_Chunks)
	{
		if (chunk)
		{
			if (chunk->m_TranslucentGPUMesh)
			{
				if (frustum.IsAABB3Inside(chunk->m_BoundingBox))
				{
					chunk->CmdDrawTranslucent(renderer, cmd, frameIndex);
				}
			}
		}
	}
	PBE::VkEndDebugUtilsLabel(g_Engine->m_pRenderer->m_Device, cmd);
}

#pragma region Lighting Processing
bool BlockWorld::HasDirtyLighting() const
{
	return m_DirtyLightSwapBuffers[0].size() > 0 || m_DirtyLightSwapBuffers[1].size() > 0;
}

void BlockWorld::MarkBlockLightDirty(BlockIterator const& blockItr)
{
	std::lock_guard<std::mutex> lock(m_DirtyLightSwapBufferMutex);
	int backBufferIndex = m_DirtyLightBufferFrameIndex == 0 ? 1 : 0;
	m_DirtyLightSwapBuffers[backBufferIndex].push_back(blockItr);
}

void BlockWorld::ProcessDirtyLighting()
{
	bool needsAnotherPass = false;
	std::vector<ChunkGenerateLightingJob*> lightingJobs = {};
	do 
	{
		std::set<Chunk*> chunksToProcess;

		for (Chunk* chunk : m_Chunks.m_Chunks)
		{
			if (chunk)
			{
				if (chunk->m_State == ChunkState::BUILDING_LIGHTING && chunk->AllBoxingNeighborsHaveBlockData())
				{
					chunksToProcess.insert(chunk);
				}
			}
		}

		for (BlockIterator& blockItr : m_DirtyLightSwapBuffers[m_DirtyLightBufferFrameIndex])
		{
			Chunk* chunk = blockItr.m_pChunk;
			if (chunk)
			{
				chunk->ProcessDirtyBlockLight(blockItr);
			}
		}

		for (Chunk* chunk : chunksToProcess)
		{
			ChunkGenerateLightingJob* job = new ChunkGenerateLightingJob(chunk);
			lightingJobs.push_back(job);
			g_Engine->m_pJobSystem->SubmitJob(job);
		}

		for (ChunkGenerateLightingJob* job : lightingJobs)
		{
			g_Engine->m_pJobSystem->WaitUntilAndClaimJobIfComplete(job);
			delete job;
		}
		lightingJobs.clear();

		for (BlockIterator& blockItr : m_DirtyLightSwapBuffers[m_DirtyLightBufferFrameIndex])
		{
			Chunk* chunk = blockItr.m_pChunk;
			if (chunk)
			{
				chunk->ProcessDirtyBlockLight(blockItr);
			}
		}

		SwapDirtyLightBuffers();
	} while ( HasDirtyLighting() );
}

void BlockWorld::SwapDirtyLightBuffers()
{
	std::lock_guard<std::mutex> lock(m_DirtyLightSwapBufferMutex);
	int backBufferIndex = m_DirtyLightBufferFrameIndex == 0 ? 1 : 0;
	m_DirtyLightSwapBuffers[m_DirtyLightBufferFrameIndex].clear();
	m_DirtyLightBufferFrameIndex = backBufferIndex;
}
#pragma endregion

#pragma region Block Get/Set
BlockIterator BlockWorld::GetBlockIterator(PBE::IntVec3 globalBlockPos)
{
	PBE::IntVec3 chunkCoords = GetChunkCoordsFromBlockCoords(globalBlockPos);
	Chunk* chunk = m_Chunks.GetChunk(chunkCoords);
	if (chunk)
	{
		PBE::IntVec3 localBlockPos = GetLocalBlockCoordsFromGlobalBlockCoords(globalBlockPos);
		return BlockIterator(chunk, GetBlockIndexFromLocalBlockCoords(localBlockPos));
	}
	return BlockIterator(nullptr, 0);
}

Block BlockWorld::GetBlock(PBE::IntVec3 globalBlockPos)
{
	PBE::IntVec3 chunkCoords = GetChunkCoordsFromBlockCoords(globalBlockPos);
	Chunk* chunk = m_Chunks.GetChunk(chunkCoords);
	if (chunk)
	{
		PBE::IntVec3 localBlockPos = GetLocalBlockCoordsFromGlobalBlockCoords(globalBlockPos);
		return chunk->GetBlock(localBlockPos);
	}
	return Block();
}

void BlockWorld::SetBlock(PBE::IntVec3 globalBlockPos, Block const& blockType)
{
	PBE::IntVec3 chunkCoords = GetChunkCoordsFromBlockCoords(globalBlockPos);
	Chunk* chunk = m_Chunks.GetChunk(chunkCoords);
	if (!chunk)
	{
		return;
	}
	if (m_NetMode == BlockWorldNetMode::SERVER)
	{
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Header.m_ID = CubikkiMessageType::SetBlock;
		msg << blockType;
		msg << globalBlockPos;

		std::cout << PBE::Stringf("[Server] Set Block at Pos %i, %i, %i", globalBlockPos.x,globalBlockPos.y,globalBlockPos.z) << "\n";
		g_Server->MessageAllClients(msg);
	}
	else
	{
		std::cout << PBE::Stringf("[Client] Set Block at Pos %i, %i, %i", globalBlockPos.x, globalBlockPos.y, globalBlockPos.z) << "\n";
		PBE::NetMessage<CubikkiMessageType> msg;
		msg.m_Header.m_ID = CubikkiMessageType::SetBlock;
		msg << blockType;
		msg << globalBlockPos;

		g_Client->Send(msg);
	}
	PBE::IntVec3 localBlockPos = GetLocalBlockCoordsFromGlobalBlockCoords(globalBlockPos);
	chunk->SetBlock(localBlockPos, blockType);
	m_ForceUpdate = true;
}
void BlockWorld::SetBlockLocally(PBE::IntVec3 globalBlockPos, Block const& blockType)
{
	PBE::IntVec3 chunkCoords = GetChunkCoordsFromBlockCoords(globalBlockPos);
	Chunk* chunk = m_Chunks.GetChunk(chunkCoords);
	if (!chunk)
	{
		return;
	}

	PBE::IntVec3 localBlockPos = GetLocalBlockCoordsFromGlobalBlockCoords(globalBlockPos);
	chunk->SetBlock(localBlockPos, blockType);
	m_ForceUpdate = true;
}
#pragma endregion

#pragma region Chunk Region Creation and Loading
bool BlockWorld::IsChunkRegionLoadingOrLoaded(PBE::IntVec3 regionPos) const
{
	if (m_ChunkRegions.find(regionPos) != m_ChunkRegions.end())
	{
		return true;
	}

	if (m_LoadChunkRegionJobsAwaitingClaim.find(regionPos) != m_LoadChunkRegionJobsAwaitingClaim.end())
	{
		return true;
	}

	return false;
}

ChunkRegion* BlockWorld::GetChunkRegion(PBE::IntVec3 regionPos)
{
	auto itr = m_ChunkRegions.find(regionPos);
	if (itr != m_ChunkRegions.end())
	{
		return itr->second;
	}
	return nullptr;
}

ChunkRegion* BlockWorld::CreateChunkRegion(PBE::IntVec3 regionPos)
{
	auto itr = m_ChunkRegions.find(regionPos);
	if (itr != m_ChunkRegions.end())
	{
		return itr->second;
	}

	ChunkRegion* region = new ChunkRegion
	(
		this, 
		m_WorldSavePath / "Region" / PBE::Stringf("%i_%i_%i.cbr",regionPos.x,regionPos.y,regionPos.z)
	);

	return region;
}
#pragma endregion

ActorUID BlockWorld::SpawnActor(std::string_view actorType, PBE::NamedProperties const& properties)
{
	return m_ActorSystem->SpawnActor(g_ActorTypesByName[actorType.data()], properties);
}

Actor* BlockWorld::GetActor(ActorUID actorUID)
{
	return m_ActorSystem->GetActor(actorUID);
}

void BlockWorld::DestroyActor(ActorUID actorUID)
{
	m_ActorSystem->DestroyActor(actorUID);
}

void BlockWorld::QueueSendChunkToClient(PBE::NetConnection<CubikkiMessageType>& client, PBE::IntVec3 chunk)
{
	if (m_NetMode == BlockWorldNetMode::SERVER)
	{
		std::lock_guard<std::mutex> lock(m_ChunkClientSendQueueMutex);
		m_ChunkClientSendQueue.push_back({ chunk, client });
	}
}

void BlockWorld::SendChunkToClient(PBE::NetConnection<CubikkiMessageType>& client, PBE::IntVec3 chunk)
{
	if (m_NetMode == BlockWorldNetMode::SERVER)
	{
		Chunk* chunkPtr = m_Chunks.GetChunk(chunk);
		if (chunkPtr)
		{
			chunkPtr->SendChunkToClient(client);
		}
	}
}

void BlockWorld::RecieveChunkFromServer(PBE::NetMessage<CubikkiMessageType>& client)
{
	if (m_NetMode == BlockWorldNetMode::CLIENT)
	{
		PBE::IntVec3 chunk;
		client >> chunk;
		Chunk* chunkPtr = m_Chunks.GetChunk(chunk);
		if (chunkPtr)
		{
			chunkPtr->RecieveChunkFromServer(client);
		}
	}
}

PBE::RaycastResult3D BlockWorld::Raycast(PBE::Vec3 const& start, PBE::Vec3 const& fwdNormal, float maxDistance)
{
	if (fwdNormal.x == 0.f && fwdNormal.y == 0.f && fwdNormal.z == 0.f)
	{
		return { false };
	}

	int blockX = (int)PBE::Floor(start.x);
	int blockY = (int)PBE::Floor(start.y);
	int blockZ = (int)PBE::Floor(start.z);

	Block block = GetBlock({ blockX, blockY, blockZ });
	BlockDef const& def = g_BlockDefs[block.m_BlockID];
	if (def.m_IsSolid)
	{
		return { true, 0.f, start, -fwdNormal, {blockX,blockY,blockZ} };
	}

	float fwdDistPerXCrossing = 1.f / abs(fwdNormal.x);
	float fwdDistPerYCrossing = 1.f / abs(fwdNormal.y);
	float fwdDistPerZCrossing = 1.f / abs(fwdNormal.z);

	int stepX = fwdNormal.x > 0 ? 1 : -1;
	int stepY = fwdNormal.y > 0 ? 1 : -1;
	int stepZ = fwdNormal.z > 0 ? 1 : -1;

	float xAtFirstCrossing = blockX + (stepX + 1.f) * 0.5f;
	float yAtFirstCrossing = blockY + (stepY + 1.f) * 0.5f;
	float zAtFirstCrossing = blockZ + (stepZ + 1.f) * 0.5f;

	float xDistToNextXCrossing = xAtFirstCrossing - start.x;
	float yDistToNextYCrossing = yAtFirstCrossing - start.y;
	float zDistToNextZCrossing = zAtFirstCrossing - start.z;

	float fwdDistAtNextXCrossing = abs(xDistToNextXCrossing) * fwdDistPerXCrossing;
	float fwdDistAtNextYCrossing = abs(yDistToNextYCrossing) * fwdDistPerYCrossing;
	float fwdDistAtNextZCrossing = abs(zDistToNextZCrossing) * fwdDistPerZCrossing;

	PBE::IntVec3 blockPos = { blockX, blockY, blockZ };
	PBE::IntVec3 localPos = GetLocalBlockCoordsFromGlobalBlockCoords(blockPos);
	int index = GetBlockIndexFromLocalBlockCoords(localPos);

	PBE::IntVec3 chunkCoords = GetChunkCoordsFromBlockCoords(blockPos);
	Chunk* chunk = m_Chunks.GetChunk(chunkCoords);

	if (!chunk)
	{
		return { false };
	}

	BlockIterator itr = BlockIterator(chunk, index);

	for (;;)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextZCrossing < fwdDistAtNextXCrossing)
			{
				if (fwdDistAtNextZCrossing > maxDistance)
				{
					return { false };
				}

				blockZ += stepZ;

				if (stepZ > 0)
				{
					itr = itr.GetUp();
				}
				else
				{
					itr = itr.GetDown();
				}

				if (!itr.IsValid())
				{
					return { false };
				}

				BlockDef* blockDef = itr.GetBlockDef();
				if (blockDef->m_IsSolid)
				{
					return 
						PBE::RaycastResult3D( true, fwdDistAtNextZCrossing, start + fwdNormal * fwdDistAtNextZCrossing, { 0.f, 0.f, -(float)stepZ }, {blockX,blockY,blockZ} );
				}
				else
				{
					fwdDistAtNextZCrossing += fwdDistPerZCrossing;
				}
			}
			else
			{
				if (fwdDistAtNextXCrossing > maxDistance)
				{
					return { false };
				}

				blockX += stepX;

				if (stepX > 0)
				{
					itr = itr.GetForward();
				}
				else
				{
					itr = itr.GetBack();
				}

				if (!itr.IsValid())
				{
					return { false };
				}

				BlockDef* blockDef = itr.GetBlockDef();
				if (blockDef->m_IsSolid)
				{
					return PBE::RaycastResult3D(true, fwdDistAtNextXCrossing, start + fwdNormal * fwdDistAtNextXCrossing, { -(float)stepX, 0.f, 0.f }, {blockX,blockY,blockZ} );
				}
				else
				{
					fwdDistAtNextXCrossing += fwdDistPerXCrossing;
				}
			}
		}
		else
		{
			if (fwdDistAtNextYCrossing < fwdDistAtNextZCrossing)
			{
				if (fwdDistAtNextYCrossing > maxDistance)
				{
					return PBE::RaycastResult3D{ false };
				}

				blockY += stepY;

				if (stepY > 0)
				{
					itr = itr.GetLeft();
				}
				else
				{
					itr = itr.GetRight();
				}

				if (!itr.IsValid())
				{
					return PBE::RaycastResult3D{ false };
				}

				BlockDef* blockDef = itr.GetBlockDef();

				if (blockDef->m_IsSolid)
				{
					return PBE::RaycastResult3D(true, fwdDistAtNextYCrossing, start + fwdNormal * fwdDistAtNextYCrossing, { 0.f,  -(float)stepY, 0.f }, { blockX,blockY,blockZ });
				}
				else
				{
					fwdDistAtNextYCrossing += fwdDistPerYCrossing;
				}
			}
			else
			{
				if (fwdDistAtNextZCrossing > maxDistance)
				{
					return PBE::RaycastResult3D{ false };
				}

				blockZ += stepZ;

				if (stepZ > 0)
				{
					itr = itr.GetUp();
				}
				else
				{
					itr = itr.GetDown();
				}

				if (!itr.IsValid())
				{
					return PBE::RaycastResult3D{ false };
				}

				BlockDef* blockDef = itr.GetBlockDef();
				if (blockDef->m_IsSolid)
				{
					return PBE::RaycastResult3D( true, fwdDistAtNextZCrossing, start + fwdNormal * fwdDistAtNextZCrossing, { 0.f, 0.f,  -(float)stepZ }, {blockX,blockY,blockZ} );
				}
				else
				{
					fwdDistAtNextZCrossing += fwdDistPerZCrossing;
				}
			}
		}
	}
}

Chunk* BlockWorld::GetChunkFromPool(PBE::IntVec3 const& coords)
{
	auto itr = std::find_if(m_ChunkPool.begin(), m_ChunkPool.end(), [this, &coords](Chunk* chunk)
		{
			if (chunk)
			{
				return true;
			}
			return false;
		}
	);

	if (itr != m_ChunkPool.end())
	{
		Chunk* chunk = *itr;
		chunk->Reset(coords, this);
		*itr = nullptr;
		return chunk;
	}

	return new Chunk(coords, this);
}

void BlockWorld::ReturnChunkToPool(Chunk* chunk)
{
	for (auto itr = m_ChunkPool.begin(); itr != m_ChunkPool.end(); ++itr)
	{
		if (!*itr)
		{
			*itr = chunk;
			return;
		}
	}

	m_ChunkPool.push_back(chunk);
}

void BlockWorldProcessLightingJob::Execute()
{
	m_BlockWorld->ProcessDirtyLighting();
}
