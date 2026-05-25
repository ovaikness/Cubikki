#pragma once

#include <vector>
#include "Engine/Math/Vec3.hpp"
#include "Engine/Graphics/Rgba8.hpp"

namespace PBE
{
	class MeshBase
	{
	public:
		virtual ~MeshBase() = default;
	public:
		std::vector<Vec3> m_Positions;
		std::vector<uint32_t> m_Indices;
	public:
	//	void AddVertsForAABB2(AABB2 bounds, AABB2 texCoords, Rgba8 color = Rgba8::WHITE);
	//	void AddVertsForOBB2(OBB2 bounds, AABB2 texCoords, Rgba8 color = Rgba8::WHITE);
	};
}