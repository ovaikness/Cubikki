#pragma once

#include "Cubikki/Actors/ActorUtils.hpp"
#include "Cubikki/Net/RemoteEventUtils.hpp"

#include "Engine/Core/EventDispatcher.hpp"
#include "Engine/Threads/JobExecutionGraph.hpp"

class ActorSystem
{
public:
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Actor Management
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	uint32_t m_SaltCounter = 0;
	uint32_t m_NetUIDCounter = 0;

	std::vector<Actor*> m_ActorsToSpawn;

	std::vector<Actor*> m_Actors;
	std::map<uint32_t, Actor*> m_ActorsByNetUID;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Events
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	PBE::EventDispatcher<Actor*> m_ActorSpawnedEvent;
	PBE::EventDispatcher<Actor*> m_ActorDestroyedEvent;

	//Remote event for when a remote event is received over the network.
	std::map<std::string, PBE::EventDispatcher<PBE::BufferReader>> m_RemoteEvents;

	PBE::EventDispatcher<PBE::Renderer*, VkCommandBuffer, uint32_t> m_OnCmdTransfer;
	PBE::EventDispatcher<PBE::Renderer*, VkCommandBuffer, uint32_t> m_OnCmdDraw;
public:
	ActorSystem();
	~ActorSystem();

	void Startup();

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Lua
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	//Register all usertypes for the ActorScene. (Components, Actor, etc.)
	PBE::Result RegisterUsertypes(PBE::LuaSystem& system);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Actor Get/Set
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	template <typename T_ActorType>
	std::vector<T_ActorType*> FindActorsOfType() const
	{
		std::vector<T_ActorType*> actors;
		for (Actor* actor : m_Actors)
		{
			if (T_ActorType* typedActor = dynamic_cast<T_ActorType*>(actor))
			{
				actors.push_back(typedActor);
			}
		}
		return actors;
	}
	Actor* GetActor(ActorUID uid) const;
	Actor* GetActorByNetUID(uint32_t netUID) const;
	ActorUID LocalSpawnActor(ActorType type, PBE::NamedProperties const& spawnInfo);
	ActorUID SpawnActor(ActorType type, PBE::NamedProperties const& spawnInfo, bool isServer = true, uint32_t givenNetUID = 0);
	void RemoteSpawnAllActorsToClient(uint32_t clientID);
	void RemoteSpawnActor(ActorType type, PBE::NamedProperties const& spawnInfo);

	void DestroyActor(ActorUID uid);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//System Updates
	//-----------------------------------------------------------------------------------------------------------------------------------------------------

	void BeginFrame();

	void Update();

	void EndFrame();

	void Clear();
private:
	void DestroyAwaitingActors();
};