#pragma once

#include <cstdint>
#include <array>
#include "Engine/Math/AABB2.hpp"
#include "Cubikki/Common.hpp"

class BlockDef
{
public:
	bool m_IsVisible{ true };
	bool m_IsSolid{ false };
	bool m_IsOpaque{ false };
	uint8_t m_BlockLightIntensity{ 0 };
	std::array<PBE::AABB2, 6> m_TextureCoords{};
	PBE::Sound* m_BreakSound{ nullptr };
	PBE::Sound* m_PlaceSound{ nullptr };
public:
	BlockDef() = default;
};