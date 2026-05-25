#pragma once
#include "Cubikki/Actors/Component/ComponentUID.hpp"
#include "Cubikki/Actors/Component/ComponentList.hpp"
#include "Cubikki/Actors/Component/ComponentReference.hpp"

template <typename T_Component>
	requires std::derived_from<T_Component, Component>
class ComponentSystem
{
public:
	ComponentList<T_Component> m_Components{};

	template<typename... Args>
	UniqueComponentReference CreateComponent(Args&&... args)
	{
		return { m_Components,m_Components.EmplaceComponent(std::forward<Args>(args)...) };
	}

	UniqueComponentReference InsertComponent(T_Component&& component)
	{
		return { m_Components,m_Components.InsertComponent(component) };
	}

	Component* GetComponent(ComponentUID uid)
	{
		return m_Components.GetComponent(uid);
	}

	T_Component* GetComponentAs(ComponentUID uid)
	{
		return m_Components.GetTypedComponent(uid);
	}

	bool HasComponent(ComponentUID uid)
	{
		return m_Components.HasComponent(uid);
	}

	void RemoveComponent(ComponentUID uid)
	{
		m_Components.RemoveComponent(uid);
	}
};