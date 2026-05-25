#pragma once

#include "Cubikki/Blocks/WorldGenerator.hpp"
#include "Cubikki/Blocks/ChunkRegion.hpp"
#include "Cubikki/Blocks/Block.hpp"

#include "Cubikki/Blocks/BlockIterator.hpp"
#include "Cubikki/Blocks/Chunk.hpp"
#include "Cubikki/Blocks/BlockCommon.hpp"
#include "Cubikki/Blocks/ChunkTable.hpp"
#include "Cubikki/Net/CubikkiServer.hpp"
#include "Cubikki/Net/CubikkiClient.hpp"
#include "Cubikki/Actors/ActorSystem.hpp"

#include "Engine/Core/EventDispatcher.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/RaycastResult3D.hpp"
#include "Engine/Graphics/Renderer.hpp"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Threads/JobSystem.hpp"
#include "Engine/Time/Timer.hpp"
#include "Engine/Scene/SkeletalModel.hpp"
#include "Engine/Core/NamedProperties.hpp"

#include <filesystem> 
#include <unordered_map>
#include <map>
#include <vector>
#include <mutex>
#include <set>

class BlockPhysicsWorld;

enum class BlockWorldNetMode
{
	OFFLINE,
	SERVER,
	CLIENT
};
enum GeneratorType : size_t
{
	WORLD_GEN_FLAT,
	WORLD_GEN_COUNT
};

class LoadChunkRegionJob;

class BlockWorld;
class BlockWorldProcessLightingJob : public PBE::Job
{
public:
	BlockWorld* m_BlockWorld;
public:
	BlockWorldProcessLightingJob(BlockWorld* blockWorld)
		: m_BlockWorld(blockWorld)
	{
	}
	void Execute() override;
};

struct ChunkClientSendPair
{
	ChunkClientSendPair(PBE::IntVec3 chunk, PBE::NetConnection<CubikkiMessageType>& client)
		: m_ChunkWorldCoords(chunk)
		, m_Client(&client)
	{
	}
	ChunkClientSendPair(ChunkClientSendPair const& other)
		: m_ChunkWorldCoords(other.m_ChunkWorldCoords)
		, m_Client(other.m_Client)
	{
	}
	PBE::IntVec3 m_ChunkWorldCoords;
	PBE::NetConnection<CubikkiMessageType>* m_Client;
};


class BlockWorld
{
public:
	BlockWorldNetMode m_NetMode = BlockWorldNetMode::OFFLINE;
	VkPipeline m_SkyboxPipeline;
	PBE::GPUBuffer* m_QuadBuffer;
	bool m_InitialLoadComplete = false;
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Players
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	std::string m_LocalPlayerName = "Player";
	std::map < std::string, PBE::IntVec3 > m_PlayerPositions;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//File IO
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	std::filesystem::path m_WorldSavePath = "World/New World";
	PBE::JobSystem* m_FileJobSystem;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Physics and Actors
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	PBE::Vec3 m_Gravity{ 0.f, 0.f , -9.8f };
	BlockPhysicsWorld* m_PhysicsWorld;
	ActorSystem* m_ActorSystem;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Chunk Management
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	//Events for when a chunk is activated or deactivated.

	//Event for when a chunk is completed, meaning its data is valid and it is renderable. Useful for actors.
	PBE::EventDispatcher<Chunk*> m_ChunkCompletedEvent;
	//Event for when a chunk is deactivated. Useful for actors.
	PBE::EventDispatcher<Chunk*> m_ChunkDeactivateEvent;
	PBE::EventDispatcher<> m_CompleteInitialLoadEvent;

	WorldGenerator* m_WorldGenerators[GeneratorType::WORLD_GEN_COUNT];
	WorldGenerator* m_WorldGenerator;

	int m_ViewDistance = 12;
	int m_NumToActivatePerFrame = 45;
	int m_NumToDeactivatePerFrame = 45;

	std::map<PBE::IntVec3, LoadChunkRegionJob*, PBE::IntVec3Comparator> m_LoadChunkRegionJobsAwaitingClaim;
	std::unordered_map<PBE::IntVec3, ChunkRegion*> m_ChunkRegions;
	std::unordered_map<PBE::IntVec3, int> m_NumChunksInRegion;
	ChunkTable m_Chunks;
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Lighting Processing
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	std::mutex m_DirtyLightSwapBufferMutex;
	int m_DirtyLightBufferFrameIndex = 0;
	std::vector<BlockIterator> m_DirtyLightSwapBuffers[2];
	std::vector<Chunk*> m_ChunkPool;

	PBE::Timer m_DebugTimer;
	PBE::Timer m_UpdateTimer;
	PBE::Timer m_RandomTickTimer;

	PBE::Material* m_Material;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Net
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	std::mutex m_ChunkClientSendQueueMutex;
	std::vector<ChunkClientSendPair> m_ChunkClientSendQueue;
public:
	BlockWorld(std::string_view fileName, CubikkiClient* client);
	BlockWorld(std::string_view fileName, CubikkiServer* server);
	BlockWorld(std::string_view fileName);
	~BlockWorld();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//#TODO Figure out where to do block def loading or how properly. (Maybe use a .lua file for each block?)
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	void LoadBlockDefs();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	void AddPlayer(std::string_view playerName, PBE::IntVec3 position);
	void SetPlayerPosition(std::string_view playerName, PBE::IntVec3 position);
	void RemovePlayer(std::string_view playerName);

	int  GetSquaredChunkDistanceToNearestPlayer(PBE::IntVec3 position) const;
	int  GetSquaredDistanceToNearestPlayer(PBE::IntVec3 position) const;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	void Update();
	void DebugPrintChunks();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Rendering
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	void CmdTransfer(PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex);
	void CmdDraw(PBE::Camera* camera, PBE::Renderer* renderer, VkCommandBuffer cmd, uint32_t frameIndex);
	void CmdDrawTranslucent(PBE::Camera* camera, PBE::Renderer* renderer, VkCommandBuffer, uint32_t frameIndex);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Light Processing
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	bool HasDirtyLighting() const;
	void MarkBlockLightDirty(BlockIterator const& blockItr);
	void ProcessDirtyLighting();
	void SwapDirtyLightBuffers();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Block Getter Stters
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	BlockIterator GetBlockIterator(PBE::IntVec3 globalBlockPos);
	void  SetBlock(PBE::IntVec3 globalBlockPos, Block const& blockType);
	void  SetBlockLocally(PBE::IntVec3 globalBlockPos, Block const& blockType);
	Block GetBlock(PBE::IntVec3 globalBlockPos);


	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Region Helpers
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	bool IsChunkRegionLoadingOrLoaded(PBE::IntVec3 regionPos) const;
	ChunkRegion* GetChunkRegion(PBE::IntVec3 regionPos);
	ChunkRegion* CreateChunkRegion(PBE::IntVec3 regionPos);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Actor spawning helpers
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	ActorUID SpawnActor(std::string_view actorType, PBE::NamedProperties const& properties);
	Actor* GetActor(ActorUID actorUID);
	template <typename T_Actor>
	T_Actor* GetActor(ActorUID actorUID)
	{
		return dynamic_cast<T_Actor*>(GetActor(actorUID));
	}
	void DestroyActor(ActorUID actorUID);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Net
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	void QueueSendChunkToClient(PBE::NetConnection<CubikkiMessageType>& client, PBE::IntVec3 chunk);
	void SendChunkToClient(PBE::NetConnection<CubikkiMessageType>& client, PBE::IntVec3 chunk);
	void RecieveChunkFromServer(PBE::NetMessage<CubikkiMessageType>& client);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//#TODO Figure out proper masking, and how to handle the raycast result. For instance entities may want to block certain raycasts.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	PBE::RaycastResult3D Raycast(PBE::Vec3 const& start, PBE::Vec3 const& fwdNormal, float maxDistance);

private:
	friend class Chunk;
	BlockWorldProcessLightingJob* m_ProcessLightingJob = nullptr;
	bool m_ForceUpdate = false;

	//Pointers to the server and client for networking. Depending on the net mode one of these will be null.
	CubikkiServer* m_Server = nullptr;
	CubikkiClient* m_Client = nullptr;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Chunk Pooling
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	Chunk* GetChunkFromPool(PBE::IntVec3 const& coords);
	void ReturnChunkToPool(Chunk* chunk);


};

class LoadChunkRegionJob : public PBE::Job
{

public:
	BlockWorld* m_BlockWorld;
	PBE::IntVec3 m_RegionPos;
	ChunkRegion* m_Region;
public:
	LoadChunkRegionJob(BlockWorld* blockWorld, PBE::IntVec3 regionPos)
		: m_BlockWorld(blockWorld)
		, m_RegionPos(regionPos)
		, m_Region(nullptr)
	{
	}

	void Execute() override
	{
		m_Region = m_BlockWorld->CreateChunkRegion(m_RegionPos);
	}
};

//Singleton reference to the block world. Should be removed in the future.
extern BlockWorld* g_BlockWorld;