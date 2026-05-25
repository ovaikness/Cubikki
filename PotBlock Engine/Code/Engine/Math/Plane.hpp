#pragma once

#include "Engine/Math/Vec3.hpp"

namespace PBE
{
	class AABB3;
	class Plane
	{
	public:
		Vec3 m_Normal;
		float m_Distance;
	public:
		~Plane();
		Plane();
		Plane(Vec3 const& normal, float distance);
		Plane(Vec3 const& normal, Vec3 const& point);

		float GetSignedDistanceToPoint(Vec3 const& point) const;
		bool IsPointInFront(Vec3 const& point) const;

		bool IsSphereInFront(Vec3 const& center, float radius) const;
		bool IsAABB3InFront(AABB3 const& aabb) const;
		bool DoesSphereIntersect(Vec3 const& center, float radius) const;
	};
}