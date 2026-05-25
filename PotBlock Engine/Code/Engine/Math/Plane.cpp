#include "Engine/Math/Plane.hpp"

#include "Engine/Math/MathUtils.hpp"

namespace PBE
{
	Plane::~Plane()
	{

	}

	Plane::Plane()
		: m_Normal(0.f, 1.f, 0.f)
		, m_Distance(0.f)
	{

	}

	Plane::Plane(Vec3 const& normal, float distance)
		: m_Normal(normal)
		, m_Distance(distance)
	{

	}

	Plane::Plane(Vec3 const& point, Vec3 const& normal)
		: m_Normal(normal.GetNormalized())
		, m_Distance(Vec3::DotProduct(m_Normal, point))
	{
	}

	float Plane::GetSignedDistanceToPoint(Vec3 const& point) const
	{
		return Vec3::DotProduct(m_Normal, point) - m_Distance;
	}

	bool Plane::IsPointInFront(Vec3 const& point) const
	{
		if (GetSignedDistanceToPoint(point) > 0.f)
		{
			return true;
		}
		return false;
	}

	bool Plane::IsSphereInFront(Vec3 const& center, float radius) const
	{
		float distance = GetSignedDistanceToPoint(center);

		if (Abs(distance) < radius)
		{
			return true;
		}

		if (distance > 0.f)
		{
			return true;
		}

		return false;
	}

	bool Plane::IsAABB3InFront(AABB3 const& aabb) const
	{
		return false;
	}

	bool Plane::DoesSphereIntersect(Vec3 const& center, float radius) const
	{
		float distance = GetSignedDistanceToPoint(center);
		if (Abs(distance) < radius)
		{
			return true;
		}
		return false;
	}

}