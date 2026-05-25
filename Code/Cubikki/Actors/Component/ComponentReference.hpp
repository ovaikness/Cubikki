#pragma once
#include "Cubikki/Actors/Component/ComponentUID.hpp"

class ComponentListBase;
class UniqueComponentReference;
class Component;

class ComponentReference
{
public:
	ComponentListBase* m_List{ nullptr };
	ComponentUID m_UID{ INVALID_COMPONENT_UID };

public:
	ComponentReference() = default;
	virtual ~ComponentReference() = default;

	ComponentReference(ComponentListBase& list, ComponentUID uid);
	ComponentReference(ComponentReference const& reference);
	ComponentReference(ComponentReference&& reference) noexcept;
	ComponentReference(UniqueComponentReference const& reference);

	ComponentReference& operator=(ComponentReference const& other);

	ComponentReference& operator=(ComponentReference&& other) noexcept;

	explicit operator Component* () const;

	Component* operator->() const;

	explicit operator bool() const;

	template<typename T_Component>
	T_Component* GetAs() const
	{
		return dynamic_cast<T_Component*>(Get());
	};

	Component* Get() const;

	bool IsValid() const;
};

class UniqueComponentReference : public ComponentReference
{
public:
	UniqueComponentReference();

	UniqueComponentReference(ComponentListBase& list, ComponentUID component);

	UniqueComponentReference(UniqueComponentReference&& other) noexcept;

	//Cannot copy an owning reference.
	UniqueComponentReference(UniqueComponentReference const& other) = delete;
	void operator=(UniqueComponentReference const& other) = delete;

	void operator=(UniqueComponentReference&& other) noexcept;

	virtual ~UniqueComponentReference();
};