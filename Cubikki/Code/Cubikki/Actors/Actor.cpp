#include "Cubikki/Actors/Actor.hpp"
#include "Cubikki/Actors/ActorSystem.hpp"
#include "Cubikki/Actors/Component/Component.hpp"
#include "Cubikki/Net/CubikkiClient.hpp"

Actor::Actor(PBE::NamedProperties const& properties)
	: m_NetProperties(properties)
{
	m_Environment = g_Engine->m_pLuaSystem->CreateEnvironment();
}

Actor::~Actor()
{
	for (auto& handle : m_RemoteEventHandles)
	{
		UnsubscribeFromRemoteEvent(handle.first, handle.second);
	}
}

ComponentReference Actor::AddComponent(UniqueComponentReference&& component)
{
	std::string name = component->m_Name;
	m_ComponentMap[name] = std::move(component);
	m_ComponentMap[name]->OnAttach(this);
	return m_ComponentMap[name];
}

UniqueComponentReference Actor::RemoveComponent(std::string_view componentName)
{
	UniqueComponentReference ref;
	ref = std::move(m_ComponentMap[componentName.data()]);
	m_ComponentMap.erase(componentName.data());
	ref->OnDetach(this);
	return ref;
}

Component* Actor::GetComponent(std::string_view name) const
{
	auto itr = m_ComponentMap.find(name.data());
	if (itr != m_ComponentMap.end())
	{
		return itr->second.Get();
	}
	return nullptr;
}

void Actor::Create()
{

}

void Actor::BeginFrame()
{
	if (m_Environment["BeginFrame"] != sol::nil)
	{
		sol::protected_function beginFrame = m_Environment["BeginFrame"];
		sol::protected_function_result result = beginFrame();
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			std::cout << what << std::endl;
		}
	}
}

void Actor::Update()
{
	if (m_Environment["Update"] != sol::nil)
	{
		sol::protected_function update = m_Environment["Update"];
		sol::protected_function_result result = update();
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			std::cout << what << std::endl;
		}
	}
}

void Actor::EndFrame()
{
	if (m_Environment["EndFrame"] != sol::nil)
	{
		sol::protected_function endFrame = m_Environment["EndFrame"];
		sol::protected_function_result result = endFrame();
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			std::cout << what << std::endl;
		}
	}
}

void Actor::ImGui()
{
}

void Actor::BindLuaScriptToActor(sol::protected_function func)
{
	sol::set_environment(m_Environment, func);

	sol::protected_function_result result = func();
	if (!result.valid())
	{
		sol::error err = result;
		std::string what = err.what();
		std::cout << what << std::endl;
	}
}

void Actor::BindLuaFunctionToRemoteEvent(std::string_view eventName, std::string_view luaFunctionName)
{
	sol::protected_function func = m_Environment[luaFunctionName.data()];
	m_RemoteEventHandles[eventName.data()] = SubscribeToRemoteEvent(eventName, [this, func](uint32_t clientID,PBE::NamedProperties& props)
		{
			sol::set_environment(m_Environment, func);
			sol::protected_function_result result = func(clientID,props);
			if (!result.valid())
			{
				sol::error err = result;
				std::string what = err.what();
				std::cout << what << std::endl;
			}
			return false;
		}
	);
}

void Actor::SpawnActor(ActorType type, PBE::NamedProperties const& properties) const
{
	if (m_System)
	{
		m_System->SpawnActor(type, properties);
	}
}

bool Actor::HasAuthority() const
{
	if (!m_IsNetActor)
	{
		return true;
	}

	if (g_Server)
	{
		if (m_NetClientID == SERVER_CLIENT_ID)
		{
			return true;
		}
	}

	if (g_Client)
	{
		if (m_NetClientID == g_Client->GetID())
		{
			return true;
		}
	}

	return false;
}