#pragma once
#include <cstdint>

constexpr uint32_t COMPONENT_UID_SALT_MASK = 0xFFFF0000;
constexpr uint32_t COMPONENT_UID_INDEX_MASK = 0x0000FFFF;
constexpr uint32_t INVALID_COMPONENT_UID = 0xFFFFFFFF;
class ComponentUID
{
public:
	uint32_t m_UID{ INVALID_COMPONENT_UID };
public:

	ComponentUID()
		: m_UID(INVALID_COMPONENT_UID)
	{
	}

	ComponentUID(uint32_t uid)
		: m_UID(uid)
	{
	}

	ComponentUID(uint16_t salt, uint16_t index)
	{
		m_UID = (salt << 16) | index;
	}

	uint16_t GetSalt() const
	{
		return (m_UID & COMPONENT_UID_SALT_MASK) >> 16;
	}

	uint16_t GetIndex() const
	{
		return (m_UID & COMPONENT_UID_INDEX_MASK);
	}

	bool IsValid() const
	{
		return m_UID != INVALID_COMPONENT_UID;
	}

	bool operator==(ComponentUID const& other) const
	{
		return m_UID == other.m_UID;
	}
};