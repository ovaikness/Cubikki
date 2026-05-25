#pragma once

#include <cstdint>

constexpr uint8_t SKY_LIGHT_BITS = 4;
constexpr uint8_t BLOCK_LIGHT_BITS = 4;

constexpr uint8_t SKY_LIGHT_SHIFT = 0;
constexpr uint8_t BLOCK_LIGHT_SHIFT = SKY_LIGHT_BITS;

constexpr uint8_t SKY_LIGHT_MASK = (1 << SKY_LIGHT_BITS) - 1;
constexpr uint8_t BLOCK_LIGHT_MASK = (1 << BLOCK_LIGHT_BITS) - 1;

constexpr uint8_t SKY_LIGHT_MAX = SKY_LIGHT_MASK;
constexpr uint8_t BLOCK_LIGHT_MAX = BLOCK_LIGHT_MASK;

struct BlockLight
{
	uint8_t m_LightLevel;

	uint8_t GetSkyLight() const
	{
		return (m_LightLevel >> SKY_LIGHT_SHIFT) & SKY_LIGHT_MASK;
	}

	uint8_t GetBlockLight() const
	{
		return (m_LightLevel >> BLOCK_LIGHT_SHIFT) & BLOCK_LIGHT_MASK;
	}

	void SetSkyLight(uint8_t light)
	{
		m_LightLevel = (m_LightLevel & ~(SKY_LIGHT_MASK << SKY_LIGHT_SHIFT)) | (light << SKY_LIGHT_SHIFT);
	}

	void SetBlockLight(uint8_t light)
	{
		m_LightLevel = (m_LightLevel & ~(BLOCK_LIGHT_MASK << BLOCK_LIGHT_SHIFT)) | (light << BLOCK_LIGHT_SHIFT);
	}
};