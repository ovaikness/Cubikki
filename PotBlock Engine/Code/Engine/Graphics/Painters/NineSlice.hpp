#pragma once

#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Math/AABB2.hpp"

#include "Engine/Graphics/Painters/Painter.hpp"

#include <string>

namespace PBE
{
	enum class NineSliceMode
	{
		TILE,
		TILE_EDGE_ONLY,
		STRETCH
	};

	enum class NineSliceTilePosition : size_t
	{
		TOP_LEFT,
		TOP_CENTER,
		TOP_RIGHT,
		MIDDLE_LEFT,
		MIDDLE_CENTER,
		MIDDLE_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_CENTER,
		BOTTOM_RIGHT,
		COUNT
	};

	struct NineSliceCreateInfo
	{
		float m_CellWidth = 1.f;
		AABB2 m_TexCoords[(size_t)NineSliceTilePosition::COUNT];
		Vec2 m_Sizes[(size_t)NineSliceTilePosition::COUNT];
		NineSliceMode m_Mode;

		void AutoSliceTexcoords(AABB2 const& texCoords, AABB2 const& centerTexCoords = AABB2(Vec2(1.f/3.f,1.f/3.f), Vec2(2.f / 3.f, 2.f / 3.f)));
	};

	class NineSlice : public Painter
	{
	public:
		NineSliceCreateInfo m_CreateInfo;
	public:
		NineSlice(NineSliceCreateInfo createInfo);
		void AddVerts(CPUMesh& mesh, AABB2 const& size, Rgba8 const& color);
	protected:
		NineSliceTilePosition GetTilePosition(int x, int y, int maxXTiles, int maxYTiles) const;
	};
}