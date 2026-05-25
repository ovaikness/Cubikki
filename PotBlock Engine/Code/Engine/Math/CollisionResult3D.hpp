#pragma once

#include "Engine/Math/Vec3.hpp"

namespace PBE
{
	struct CollisionResult3D
	{
		bool m_Hit = false;
		Vec3 m_HitPosition = Vec3::ZERO;
		Vec3 m_HitSurfaceNormal = Vec3::ZERO;
		Vec3 m_HitPushDisplacement = Vec3::ZERO;

		bool m_DoesFirstPenetrateSecond = false;
		Vec3 m_FirstPentratingPoint  = Vec3::ZERO;

		bool m_DoesSecondPenetrateFirst = false;
		Vec3 m_SecondPentratingPoint = Vec3::ZERO;
	};
}