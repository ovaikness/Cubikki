#pragma once

#include "Cubikki/Actors/Component/Component.hpp"
#include <vector>
#include <cstdint>

template <typename T_Component>
	requires std::derived_from<T_Component, Component>
struct ComponentDataEntry
{
	bool         m_InUse{ false };
	T_Component  m_Component;

	ComponentDataEntry(bool inUse, T_Component&& comp)
		: m_InUse(inUse), m_Component(std::move(comp)) {
	}

	ComponentDataEntry(bool inUse, T_Component const& comp)
		: m_InUse(inUse), m_Component(comp) {
	}
};

class ComponentListBase
{
public:
	virtual ~ComponentListBase() = default;
	virtual Component* GetComponent(ComponentUID uid) = 0;
	virtual void RemoveComponent(ComponentUID uid) = 0;
	virtual bool HasComponent(ComponentUID uid) = 0;
	virtual void Clear() = 0;
};

template <typename T_Component>
	requires std::derived_from<T_Component, Component>
class ComponentList : public ComponentListBase
{
public:
	std::vector<ComponentDataEntry<T_Component>> m_Entries{};
public:
	template <typename ...Args>
	ComponentUID EmplaceComponent(Args&&... args)
	{
		ComponentUID uid = InsertComponent(std::move(T_Component(std::forward<Args>(args)...)));
		return uid;
	}

	ComponentUID InsertComponent(T_Component&& component)
	{
		static int salt = 0;
		int index;

		for (index = 0; index < m_Entries.size(); ++index)
		{
			ComponentDataEntry<T_Component>& entry = m_Entries[index];
			if (!entry.m_InUse)
			{
				entry.m_InUse = true;
				entry.m_Component = std::move(component);
				entry.m_Component.m_UID = ComponentUID(salt, index);
				return entry.m_Component.m_UID;
			}
		}

		ComponentDataEntry<T_Component>& entryRef = m_Entries.emplace_back(true, std::move(component));
		entryRef.m_Component.m_UID = ComponentUID(salt++, index);

		return entryRef.m_Component.m_UID;
	}

	ComponentDataEntry<T_Component>* FindEntry(ComponentUID uid)
	{
		if (!uid.IsValid())
		{
			return nullptr;
		}

		int index = uid.GetIndex();
		if (index >= m_Entries.size())
		{
			return nullptr;
		}

		ComponentDataEntry<T_Component>& entry = m_Entries[index];
		// If the entry is not in use, then it is garbage data.
		if (!entry.m_InUse)
		{
			return nullptr;
		}

		if (entry.m_Component.GetUID() == uid)
		{
			return &entry;
		}

		return nullptr;
	}

	T_Component* GetTypedComponent(ComponentUID uid)
	{
		ComponentDataEntry<T_Component>* entry = FindEntry(uid);
		if (entry)
		{
			return &entry->m_Component;
		}
		return nullptr;
	}

	Component* GetComponent(ComponentUID uid)
	{
		return (Component*)GetTypedComponent(uid);
	}

	void RemoveComponent(ComponentUID uid)
	{
		ComponentDataEntry<T_Component>* entry = FindEntry(uid);
		if (entry)
		{
			entry->m_Component.~T_Component();
			entry->m_InUse = false;
		}
	}

	bool HasComponent(ComponentUID uid)
	{
		return FindEntry(uid) != nullptr;
	}

	void Clear()
	{
		for (ComponentDataEntry<T_Component>& entry : m_Entries)
		{
			entry.m_InUse = false;
		}
	}

	size_t Size()
	{
		return m_Entries.size();
	}

	std::vector<ComponentDataEntry<T_Component>>::iterator begin()
	{
		return m_Entries.begin();
	}

	std::vector<ComponentDataEntry<T_Component>>::iterator end()
	{
		return m_Entries.end();
	}
};