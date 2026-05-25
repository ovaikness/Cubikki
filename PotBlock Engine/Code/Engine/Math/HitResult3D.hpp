#pragma once
#include "Engine/Math/Vec3.hpp"

namespace PBE
{
	struct HitResult3D
	{
		Vec3 m_HitPoint;
		Vec3 m_HitNormal;
		float m_HitDistance;
		bool m_Hit;
	};
}