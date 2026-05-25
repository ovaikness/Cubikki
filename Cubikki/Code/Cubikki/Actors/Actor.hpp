#pragma once

#include "Cubikki/Actors/ActorUID.hpp"
#include "Cubikki/Actors/ActorType.hpp"
#include "Cubikki/Actors/GameTag/GameTagCollection.hpp"

#include "Cubikki/Actors/Component/Component.hpp"
#include "Cubikki/Actors/Component/ComponentReference.hpp"

#include "Cubikki/Net/CubikkiMessage.hpp"
#include "Cubikki/Net/RemoteEventUtils.hpp"

#include "Cubikki/Common.hpp"

#include "Engine/Scene/Node.hpp"
#include "Engine/Scripting/LuaSystem.hpp"

#include "Engine/Graphics/UI/Canvas.hpp"
#include "Engine/Graphics/UI/Widget.hpp"
#include "Engine/Graphics/UI/Panel.hpp"
#include "Engine/Graphics/UI/TextEntryPanel.hpp"

#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/BufferReader.hpp"
#include "Engine/Core/GrowBuffer.hpp"
#include "Engine/Core/EventDispatcher.hpp"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <variant>

class BlockWorld;
class ActorSystem;

class Actor
{
public:
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Core
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	BlockWorld* m_World{ nullptr };
	ActorSystem* m_System{ nullptr };
	bool m_IsNetActor{ false };
	bool m_AwaitingCreate{ true };
	bool m_AwaitingDestroy{ false };
	sol::environment m_Environment;
	ActorType m_ActorType{ INVALID_ACTOR_TYPE };
	uint32_t m_NetUID{ 0 };
	uint32_t m_NetClientID{ 0 };
	ActorUID m_UID{ INVALID_UID };

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//User Properties
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	std::string m_name{ "Actor" };
	PBE::NamedProperties m_NetProperties;

	std::map<std::string, UniqueComponentReference> m_ComponentMap;
	std::map<std::string, PBE::SubscriberHandle<uint32_t,PBE::NamedProperties&>> m_RemoteEventHandles;
	GameTagCollection m_Tags;
public:
	Actor(PBE::NamedProperties const& properties);
	virtual ~Actor();

	bool operator==(Actor const& other) const 
	{
		return m_UID == other.m_UID;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Component Helpers
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Add a component to the actor. This is used to keep track of components that need to be referenced by the actor and cleaned up when the actor is destroyed.
	ComponentReference AddComponent(UniqueComponentReference&& component);
	UniqueComponentReference RemoveComponent(std::string_view name);

	Component* GetComponent(std::string_view name) const;

	template<typename T_Component>
		requires std::derived_from<T_Component, Component>
	T_Component* GetComponentOfType(std::string_view name) const {
		auto itr = m_ComponentMap.find(name.data());
		if (itr != m_ComponentMap.end())
		{
			return itr->second.GetAs<T_Component>();
		}
		return nullptr;
	}

	template<typename T_Component>
		requires std::derived_from<T_Component, Component>
	T_Component* FindFirstComponentOfType() const
	{
		for (auto& component : m_ComponentMap)
		{
			if (T_Component* typedComponent = component.second.GetAs<T_Component>())
			{
				return typedComponent;
			}
		}
		return nullptr;
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Built-in Events
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void Create();
	virtual void BeginFrame();
	virtual void Update();
	virtual void EndFrame();
	virtual void ImGui();

	bool HasAuthority() const;

	void Destroy()
	{
		m_AwaitingDestroy = true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//UID Helpers
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	size_t GetIndex() const { return m_UID.GetIndex(); }
	uint32_t GetSalt() const { return m_UID.GetSalt(); }
	ActorUID GetUID() const { return m_UID; }

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Lua Init
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	void BindLuaScriptToActor(sol::protected_function func);
	void BindLuaFunctionToRemoteEvent(std::string_view eventName, std::string_view luaFunctionName);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	//Properties
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	virtual void SetProperties(PBE::NamedProperties const& properties)
	{
		m_NetProperties = properties;
	}
	virtual PBE::NamedProperties GetProperties()
	{
		return m_NetProperties;
	}

	void SpawnActor(ActorType type, PBE::NamedProperties const& properties = PBE::NamedProperties()) const;
};