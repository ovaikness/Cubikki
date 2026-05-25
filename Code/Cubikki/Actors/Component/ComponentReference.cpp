#include "Cubikki/Actors/Component/ComponentReference.hpp"
#include "Cubikki/Actors/Component/ComponentList.hpp"
#include "Cubikki/Actors/Component/Component.hpp"

ComponentReference::ComponentReference(ComponentListBase& list, ComponentUID uid)
	: m_List(&list), m_UID(uid) {
}

ComponentReference::ComponentReference(ComponentReference const& reference)
	: m_List(reference.m_List), m_UID(reference.m_UID) {
}

ComponentReference::ComponentReference(ComponentReference&& reference) noexcept
	: m_List(reference.m_List), m_UID(reference.m_UID)
{
	reference.m_List = nullptr;
	reference.m_UID = INVALID_COMPONENT_UID;
}

ComponentReference::ComponentReference(UniqueComponentReference const& reference)
	: m_List(reference.m_List), m_UID(reference.m_UID)
{
}

ComponentReference& ComponentReference::operator=(ComponentReference const& other)
{
	if (this != &other)
	{
		m_List = other.m_List;
		m_UID = other.m_UID;
	}
	return *this;
}

ComponentReference& ComponentReference::operator=(ComponentReference&& other) noexcept
{
	if (this != &other)
	{
		m_List = other.m_List;
		m_UID = other.m_UID;
		other.m_List = nullptr;
		other.m_UID = INVALID_COMPONENT_UID;
	}
	return *this;
}

ComponentReference::operator Component* () const
{
	return Get();
}

Component* ComponentReference::operator->() const
{
	return Get();
}

ComponentReference::operator bool() const
{
	return IsValid();
}

Component* ComponentReference::Get() const
{
	return m_List ? m_List->GetComponent(m_UID) : nullptr;
}

bool ComponentReference::IsValid() const
{
	return m_UID.IsValid() && Get() != nullptr;
}

UniqueComponentReference::UniqueComponentReference()
	: ComponentReference()
{
}

UniqueComponentReference::UniqueComponentReference(ComponentListBase& list, ComponentUID component)
	: ComponentReference(list, component)
{
}

UniqueComponentReference::UniqueComponentReference(UniqueComponentReference&& other) noexcept
{
	*this = std::move(other);
}

void UniqueComponentReference::operator=(UniqueComponentReference&& other) noexcept
{
	this->m_UID = other.m_UID;
	this->m_List = other.m_List;
	other.m_UID = INVALID_COMPONENT_UID;
	other.m_List = nullptr;
}

UniqueComponentReference::~UniqueComponentReference()
{
	if (this->IsValid())
	{
		this->m_List->RemoveComponent(this->m_UID);
	}
};