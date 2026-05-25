#include "Engine/Graphics/Painters/NineSlice.hpp"
#include "Engine/Math/MathUtils.hpp"
PBE::NineSlice::NineSlice(NineSliceCreateInfo createInfo)
	: m_CreateInfo(createInfo)
{
}

//#TODO : Support multi sized nine slice. Each size right now is the same.
void PBE::NineSlice::AddVerts(CPUMesh& mesh, AABB2 const& bounds, Rgba8 const& color) {
	auto realDims = bounds.GetDimensions();

	int numXTiles = 2;
	int numYTiles = 2;

	float xSizeAccum = 
		m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_LEFT].x 
		+ m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_RIGHT].x;

	float ySizeAccum = 
		m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_LEFT].y
		+ m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::TOP_LEFT].y;

	while (xSizeAccum < realDims.x) {
		xSizeAccum += m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_CENTER].x;
		++numXTiles;
	}
	--numXTiles;
	xSizeAccum -= m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_CENTER].x;

	while (ySizeAccum < realDims.y) {
		ySizeAccum += m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::MIDDLE_LEFT].y;
		++numYTiles;
	}
	--numYTiles;
	ySizeAccum -= m_CreateInfo.m_Sizes[(size_t)NineSliceTilePosition::MIDDLE_LEFT].y;

	Vec2 dims = Vec2(xSizeAccum, ySizeAccum);
	Vec2 startPen = bounds.m_Mins + (realDims - dims) * 0.5f; // Center alignment for the grid

	if (m_CreateInfo.m_Mode == NineSliceMode::TILE) {
		Vec2 pen = startPen; // Initialize pen at the starting point
		for (int i = 0; i < numYTiles; ++i) {
			pen.x = startPen.x; // Reset x-coordinate for each row
			for (int j = 0; j < numXTiles; ++j) {
				NineSliceTilePosition tilePosition = GetTilePosition(j, i, numXTiles, numYTiles);
				Vec2 cellSize = m_CreateInfo.m_Sizes[(size_t)tilePosition];
				AABB2 cell(0, 0, cellSize.x, cellSize.y);
				cell.Translate(pen); // Place the cell at the current pen position
				AABB2 texCoords = m_CreateInfo.m_TexCoords[(size_t)tilePosition];
				mesh.AddVertsForAABB2(cell, texCoords, color);
				pen.x += cellSize.x; // Advance pen position horizontally
			}
			pen.y += m_CreateInfo.m_Sizes[(size_t)GetTilePosition(0, i, numXTiles, numYTiles)].y; // Advance pen position vertically
		}
	}
	else if (m_CreateInfo.m_Mode == NineSliceMode::STRETCH)
	{
		Vec2 pen = startPen; // Initialize pen at the starting point
		for (int i = 0; i < numYTiles; ++i) {
			pen.x = startPen.x; // Reset x-coordinate for each row
			for (int j = 0; j < numXTiles; ++j) {
				NineSliceTilePosition tilePosition = GetTilePosition(j, i, numXTiles, numYTiles);
				Vec2 cellSize = m_CreateInfo.m_Sizes[(size_t)tilePosition];
				AABB2 cell(0, 0, cellSize.x, cellSize.y);
				cell.Translate(pen); // Place the cell at the current pen position
				mesh.AddVertsForAABB2(cell, m_CreateInfo.m_TexCoords[(size_t)tilePosition], color);
				pen.x += cellSize.x; // Advance pen position horizontally
			}
			pen.y += m_CreateInfo.m_Sizes[(size_t)GetTilePosition(0, i, numXTiles, numYTiles)].y; // Advance pen position vertically
		}
	}
}

PBE::NineSliceTilePosition PBE::NineSlice::GetTilePosition(int x, int y, int maxXTiles, int maxYTiles) const {
	if (y == 0) {
		if (x == 0) return NineSliceTilePosition::TOP_LEFT;
		if (x == maxXTiles - 1) return NineSliceTilePosition::TOP_RIGHT;
		return NineSliceTilePosition::TOP_CENTER;
	}
	else if (y == maxYTiles - 1) {
		if (x == 0) return NineSliceTilePosition::BOTTOM_LEFT;
		if (x == maxXTiles - 1) return NineSliceTilePosition::BOTTOM_RIGHT;
		return NineSliceTilePosition::BOTTOM_CENTER;
	}
	else {
		if (x == 0) return NineSliceTilePosition::MIDDLE_LEFT;
		if (x == maxXTiles - 1) return NineSliceTilePosition::MIDDLE_RIGHT;
		return NineSliceTilePosition::MIDDLE_CENTER;
	}
}

void PBE::NineSliceCreateInfo::AutoSliceTexcoords(AABB2 const& texCoords, AABB2 const& normalizedCenterTexCoords)
{
	// Texture corners
	Vec2 tl = Vec2(texCoords.m_Mins.x, texCoords.m_Maxs.y);
	Vec2 br = Vec2(texCoords.m_Maxs.x, texCoords.m_Mins.y);
	Vec2 tr = texCoords.m_Maxs;
	Vec2 bl = texCoords.m_Mins;

	// Center-mapped corners
	Vec2 ctl = Vec2(
		RangeMap(normalizedCenterTexCoords.m_Mins.x, 0.f, 1.f, texCoords.m_Mins.x, texCoords.m_Maxs.x),
		RangeMap(normalizedCenterTexCoords.m_Maxs.y, 0.f, 1.f, texCoords.m_Mins.y, texCoords.m_Maxs.y)
	);

	Vec2 cbr = Vec2(
		RangeMap(normalizedCenterTexCoords.m_Maxs.x, 0.f, 1.f, texCoords.m_Mins.x, texCoords.m_Maxs.x),
		RangeMap(normalizedCenterTexCoords.m_Mins.y, 0.f, 1.f, texCoords.m_Mins.y, texCoords.m_Maxs.y)
	);

	Vec2 ctr = Vec2(
		RangeMap(normalizedCenterTexCoords.m_Maxs.x, 0.f, 1.f, texCoords.m_Mins.x, texCoords.m_Maxs.x),
		RangeMap(normalizedCenterTexCoords.m_Maxs.y, 0.f, 1.f, texCoords.m_Mins.y, texCoords.m_Maxs.y)
	);

	Vec2 cbl = Vec2(
		RangeMap(normalizedCenterTexCoords.m_Mins.x, 0.f, 1.f, texCoords.m_Mins.x, texCoords.m_Maxs.x),
		RangeMap(normalizedCenterTexCoords.m_Mins.y, 0.f, 1.f, texCoords.m_Mins.y, texCoords.m_Maxs.y)
	);

	m_TexCoords[(size_t)NineSliceTilePosition::BOTTOM_LEFT] = AABB2(bl, cbl);
	m_TexCoords[(size_t)NineSliceTilePosition::BOTTOM_CENTER] = AABB2(Vec2(cbl.x, bl.y), cbr);
	m_TexCoords[(size_t)NineSliceTilePosition::BOTTOM_RIGHT] = AABB2(Vec2(cbr.x, br.y), Vec2(br.x, cbr.y));

	m_TexCoords[(size_t)NineSliceTilePosition::MIDDLE_LEFT] = AABB2(Vec2(bl.x, cbl.y), ctl);
	m_TexCoords[(size_t)NineSliceTilePosition::MIDDLE_CENTER] = AABB2(cbl, cbr);
	m_TexCoords[(size_t)NineSliceTilePosition::MIDDLE_RIGHT] = AABB2(cbr, Vec2(br.x, ctr.y));

	m_TexCoords[(size_t)NineSliceTilePosition::TOP_LEFT] = AABB2(Vec2(bl.x, ctl.y), Vec2(cbl.x, tl.y));
	m_TexCoords[(size_t)NineSliceTilePosition::TOP_CENTER] = AABB2(ctl, Vec2(ctr.x, tr.y));
	m_TexCoords[(size_t)NineSliceTilePosition::TOP_RIGHT] = AABB2(ctr, tr);

	// Calculate sizes based on texture coordinates
	m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_LEFT] = cbl - bl;//
	m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_CENTER] = cbr - Vec2(cbl.x,bl.y);//
	m_Sizes[(size_t)NineSliceTilePosition::BOTTOM_RIGHT] = Vec2(br.x,cbr.y) - Vec2(cbr.x, br.y);//

	m_Sizes[(size_t)NineSliceTilePosition::MIDDLE_LEFT] = ctl - Vec2(bl.x,cbl.y);//
	m_Sizes[(size_t)NineSliceTilePosition::MIDDLE_CENTER] = ctr - cbl;//
	m_Sizes[(size_t)NineSliceTilePosition::MIDDLE_RIGHT] = Vec2(tr.x,ctr.y) - cbr;//

	m_Sizes[(size_t)NineSliceTilePosition::TOP_LEFT] = Vec2(ctl.x, tl.y) - Vec2(tl.x,ctl.y);
	m_Sizes[(size_t)NineSliceTilePosition::TOP_CENTER] = Vec2(ctr.x,tr.y) - ctl;
	m_Sizes[(size_t)NineSliceTilePosition::TOP_RIGHT] = tr - ctr;

	for (size_t i = 0; i < (size_t)NineSliceTilePosition::COUNT; ++i) {
		m_Sizes[i].x = RangeMap(m_Sizes[i].x, 0.f, br.x - bl.x, 0.f, m_CellWidth * 3.f);
		m_Sizes[i].y = RangeMap(m_Sizes[i].y, 0.f, tl.y - bl.y, 0.f, m_CellWidth * 3.f);
	}
}
