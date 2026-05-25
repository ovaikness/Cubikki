#pragma once

#include <cstdint>

constexpr uint32_t SALT_MASK = 0xFFFF0000;
constexpr uint32_t INDEX_MASK = 0x0000FFFF;
constexpr uint32_t INVALID_UID = 0xFFFFFFFF;
class ActorUID
{
public:
	uint32_t m_UID{ INVALID_UID };
public:

	ActorUID()
		: m_UID(INVALID_UID)
	{
	}

	ActorUID(uint32_t uid)
		: m_UID(uid)
	{
	}

	ActorUID(uint16_t salt, uint16_t uid)
	{
		m_UID = (salt << 16) | uid;
	}

	uint16_t GetSalt() const
	{
		return (m_UID & SALT_MASK) >> 16;
	}

	uint16_t GetIndex() const
	{
		return (m_UID & INDEX_MASK);
	}

	bool IsValid() const
	{
		return m_UID != INVALID_UID;
	}

	bool operator==(ActorUID const& other) const
	{
		return m_UID == other.m_UID;
	}
};