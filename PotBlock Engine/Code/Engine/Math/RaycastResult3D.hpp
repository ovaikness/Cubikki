#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec3.hpp"
namespace PBE
{
	struct RaycastResult3D
	{
		bool m_Hit{ false };
		float m_HitDistance{ 0.f };
		Vec3 m_HitPoint{};
		Vec3 m_HitNormal{};

		IntVec3 m_BlockPos{};
	};
}