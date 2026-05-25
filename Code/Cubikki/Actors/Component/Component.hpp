#pragma once
#include "Cubikki/Actors/Component/ComponentUID.hpp"
#include "Cubikki/Actors/Component/ComponentReference.hpp"

#include <map>
#include <string>
#include <vector>

class Actor;

class Component
{
public:
	bool m_HideInEditor = false;
	ComponentUID m_UID;
	ComponentReference m_Parent;
	UniqueComponentReference m_TestChild;
	Actor* m_pOwningActor{ nullptr };
	std::string m_Name;
public:
	Component() = default;
	Component(std::string_view name)
		: m_Name(name)
	{
	};
	Component(Component const& component) = delete;
	Component(Component&& component) noexcept
	{
		m_UID = component.m_UID;
		m_Parent = component.m_Parent;
		m_TestChild = std::move(component.m_TestChild);
		m_pOwningActor = component.m_pOwningActor;
		m_Name = std::move(component.m_Name);
	}
	Component& operator=(Component const& other) = delete;
	Component& operator=(Component&& other) noexcept
	{
		m_UID = other.m_UID;
		m_Parent = other.m_Parent;
		m_TestChild = std::move(other.m_TestChild);
		m_pOwningActor = other.m_pOwningActor;
		m_Name = std::move(other.m_Name);
		return *this;
	}
	virtual ~Component() = default;

	Actor* GetOwner() const
	{
		return m_pOwningActor;
	}

	ComponentUID GetUID() const
	{
		return m_UID;
	}
	virtual void ImGui() = 0;
	virtual void OnAttach(Actor* actor) {
		m_pOwningActor = actor;
	};
	virtual void OnDetach(Actor*) {
	};

	//void AddChild(UniqueComponentReference&& child)
	//{
	//	m_Children[child->m_Name] = std::move(child);
	//};
	//
	//void RemoveChild(std::string_view name)
	//{
	//	m_Children.erase(name.data());
	//};
};